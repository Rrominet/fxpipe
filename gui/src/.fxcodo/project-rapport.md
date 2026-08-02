[CRITICAL] App_impl.cpp — line containing `clipboard->read_text_async({});`
Problem: `read_text_async()` starts an asynchronous clipboard read, but its result is discarded. `read_text_finish({})` is then called with an empty/invalid `GAsyncResult`.

Why it matters: This directly causes `gdk_clipboard_read_text_finish: assertion 'g_task_is_valid (result, clipboard)' failed`. The clipboard read has not completed, and `finish()` receives no valid task result. The function will not work reliably and may return invalid data or trigger GTK critical errors.

Fix: Make clipboard reading asynchronous and process the text inside the completion callback. `pastFromClipboard()` must also become asynchronous, or the clipboard implementation must expose a synchronous read API.

```cpp
void App_impl::clipboardText(std::function<void(const std::string&)> callback) const
{
    auto clipboard = _gtkapp->get_windows()[0]->get_display()->get_clipboard();

    clipboard->read_text_async(
        {},
        [clipboard, callback](GAsyncResult* result)
        {
            try
            {
                auto text = clipboard->read_text_finish(result);
                callback(text ? text : "");
            }
            catch (const std::exception& e)
            {
                callback("");
            }
        });
}
```

Then call `pastTasks()` from the callback after parsing the returned text.

---

[HIGH] FxPipe.cpp — line containing `auto data_s = this->clipboardText();`
Problem: `pastFromClipboard()` assumes clipboard access is synchronous and returns a completed string immediately.

Why it matters: There is no valid synchronous result available after `read_text_async()`. Even if `read_text_finish()` were called with the correct result later, the current function has already returned, so parsing and pasting cannot remain in this linear form.

Fix: Change the control flow to callback-based processing:

```cpp
void FxPipe::pastFromClipboard(TaskView* view)
{
    this->clipboardText([this, view](const std::string& data_s)
    {
        if (data_s.empty())
            return;

        try
        {
            json data = json::parse(data_s);
            this->pastTasks(data, view);
        }
        catch (const std::exception& e)
        {
            this->error("Error while pasting from clipboard : " + _S(e.what()));
        }
    });
}
```

The callback must run on the GUI thread, and `view` must still be valid when it runs.

---

[HIGH] App_impl.cpp — line containing `_gtkapp->get_windows()[0]`
Problem: The code unconditionally indexes the first GTK window.

Why it matters: If the application has no windows at that moment, `get_windows()[0]` is out of bounds. Clipboard access will then invoke undefined behavior instead of producing a controlled error.

Fix: Check that the window list is non-empty before indexing it, or obtain the display directly from the application instead of deriving it from window zero.

---

[MEDIUM] App_impl.cpp — line containing `clipboard->read_text_finish({});`
Problem: The completion result is not passed from the asynchronous operation to the finish function.

Why it matters: GTK asynchronous APIs require the exact `GAsyncResult*` produced by the corresponding operation. Constructing `{}` does not represent completion and cannot be used with `read_text_finish()`.

Fix: Store/use the `GAsyncResult*` supplied to the async completion callback:

```cpp
clipboard->read_text_async(
    {},
    [clipboard](GAsyncResult* result)
    {
        auto text = clipboard->read_text_finish(result);
    });
```

Use the actual callback signature required by the wrapper/library in this codebase.

---

[MEDIUM] FxPipe.cpp — line containing the asynchronous clipboard callback
Problem: The callback can outlive `FxPipe` or the supplied `TaskView*`.

Why it matters: Capturing raw `this` and `view` creates a use-after-free risk if the window closes, the application shuts down, or the task view is destroyed before GTK completes the clipboard read.

Fix: Cancel pending clipboard reads during shutdown, or capture a lifetime-safe owner and resolve the current `TaskView` only when the callback executes. Do not retain a raw `TaskView*` across an asynchronous boundary unless its lifetime is guaranteed.