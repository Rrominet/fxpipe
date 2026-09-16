[CRITICAL] TimeTrackingWin.cpp — line 296  
Problem: The interval callback calls `stopTrack()` when `_dataFromIndex(_beingTracked)` returns `nullptr`, then immediately dereferences `d`.

Why it matters: `stopTrack()` does not return from the callback. If the tracked session was removed or reloaded, the next interval tick executes `d->duration_done` on a null pointer. Redrawing can expose this when the underlying session indices become invalid.

Fix:
```cpp
auto d = _dataFromIndex(_beingTracked);
if (!d)
{
    this->stopTrack();
    return;
}
```

---

[HIGH] TimeTrackingWin.cpp — lines 285–316  
Problem: Tracking stores the session position in `_beingTracked`, not a stable session pointer. Any removal or reload can change the vector indices while the interval is active.

Why it matters: The interval can start tracking one session and later update a different session, or receive an invalid index and crash. `removeSessionFromData()` and `load()` do not stop tracking or repair `_beingTracked`.

Fix: Stop tracking before modifying `_sessions`, and reset the tracking index:
```cpp
void TimeTrackingWin::load()
{
    stopTrack();
    auto data = storage::get<json>("time-tracking-sessions");
    deserializeSessions(data);
}

void TimeTrackingWin::removeSessionFromData(SessionData* sessionData)
{
    if (_beingTracked == _indexFromData(sessionData))
        stopTrack();

    // remove session...
}
```
Better: track a `SessionData*` and validate it before every update.

---

[HIGH] TimeTrackingWin.cpp — line 300  
Problem: `1/(60*60)` is integer division and evaluates to `0`.

Why it matters: `duration_done` never increases. The interval redraws every second forever, and completion is never reached. This is a definite logic bug.

Fix: Store elapsed time in seconds:
```cpp
d->duration_done += 1;
```
Then compare against a duration also stored in seconds. If the UI duration is in hours, convert it when creating the session:
```cpp
s->duration = static_cast<unsigned int>(_sessionGui.duration.value() * 60.0f * 60.0f);
```

---

[HIGH] TimeTrackingWin.cpp — lines 296–307  
Problem: The interval callback calls `redrawSessions()`, which destroys every `SessionDrawn` object and recreates them every second.

Why it matters: This is unnecessary destruction/recreation of UI objects while an interval callback is running. Any framework event, property callback, or pending UI operation referring to the old widgets can dereference freed objects. It is a likely source of the redraw crash and is poor design even when it does not crash.

Fix: Update only the active session's duration label and completion CSS. Do not rebuild the entire session list on every timer tick:
```cpp
d->duration_done += 1;

auto drawn = _fromIndex(_activeSession);
if (drawn && drawn->data == d)
{
    drawn->duration->setText(
        math::round(d->duration_done) + "/" +
        math::round(d->duration)
    );
}
```
Use the actual label-update API provided by `ml::Label`.

---

[MEDIUM] TimeTrackingWin.cpp — lines 251–258  
Problem: `setActiveSession()` updates `_activeSession` using `_indexFromSessionDrawn()`, which returns an index into `_sessionsDrawn`, while tracking uses `_beingTracked` as an index into `_sessions`.

Why it matters: These vectors usually have matching order, but they are separate containers and are rebuilt independently. Their indices are being treated as interchangeable. Any filtering, partial redraw, or future change breaks that assumption and updates the wrong session.

Fix: Use the `SessionData*` directly:
```cpp
void TimeTrackingWin::setActiveSession(SessionDrawn* sessionDrawn)
{
    _activeSession = _indexFromData(sessionDrawn ? sessionDrawn->data : nullptr);

    for (auto& s : _sessionsDrawn)
        s->box->removeCssClass("active");

    if (sessionDrawn)
        sessionDrawn->box->addCssClass("active");
}
```

---

[MEDIUM] TimeTrackingWin.cpp — lines 228–245  
Problem: The `done` property callback captures a raw `SessionDrawn*`.

Why it matters: `redrawSessions()` destroys all `SessionDrawn` instances. If the property system invokes a queued or delayed update after destruction, the callback dereferences a dangling pointer through `ptr->data` and `ptr->box`.

Fix: Do not capture the owning draw object in a callback that can outlive it. Capture only stable data and ensure the callback is disconnected before destruction, or avoid rebuilding the widget tree while callbacks are active.

---

[MEDIUM] TimeTrackingWin.cpp — lines 122–124  
Problem: Reloading calls `load()` without clearing or stopping the active tracking state.

Why it matters: `_sessions` is cleared, invalidating every `SessionData*` previously used by the UI and potentially by the active interval. The next interval callback can use an invalid index or null pointer. The UI is also not redrawn after reload.

Fix:
```cpp
void TimeTrackingWin::load()
{
    stopTrack();
    _beingTracked = -1;
    _activeSession = -1;

    auto data = storage::get<json>("time-tracking-sessions");
    deserializeSessions(data);
    redrawSessions();
}
```
Do not call `stopTrack()` if it emits an error when no timer exists; make it safely idempotent.

---

[LOW] TimeTrackingWin.cpp — lines 318–330  
Problem: `stopTrack()` reports `"No tracking started."` when called defensively after tracking has already stopped.

Why it matters: Once null-session handling is fixed, calling `stopTrack()` from cleanup paths becomes normal. Logging it as an error makes expected state transitions look like failures.

Fix:
```cpp
void TimeTrackingWin::stopTrack()
{
    if (_trackIntervalId == -1)
        return;

    ml::app()->removeInterval(_trackIntervalId);
    _trackIntervalId = -1;
    _beingTracked = -1;
    this->setInfos("Tracking stopped.", 3000);
}
```