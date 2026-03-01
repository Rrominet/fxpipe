//This is a generated file, don't change it manually, it will be override when rebuild.

std::string id(){return _id;}
const std::string& id() const {return _id;}


std::string category(){return _category;}
const std::string& category() const {return _category;}
void setCategory(const std::string& category){_category = category;}

std::string name(){return _name;}
const std::string& name() const {return _name;}
void setName(const std::string& name){_name = name;}

std::string description(){return _description;}
const std::string& description() const {return _description;}
void setDescription(const std::string& description){_description = description;}

bool archived(){return _archived;}
const bool archived() const {return _archived;}
void setArchived(bool archived){_archived = archived;}

int64_t creationTime(){return _creationTime;}
const int64_t creationTime() const {return _creationTime;}
void setCreationTime(int64_t creationTime){_creationTime = creationTime;}

int64_t deadline(){return _deadline;}
const int64_t deadline() const {return _deadline;}
void setDeadline(int64_t deadline){_deadline = deadline;}

Color<float>& color(){return _color;}
const Color<float>& color() const {return _color;}
void setColor(const Color<float>& color){_color = color;}

Status& status(){return _status;}
const Status& status() const {return _status;}
void setStatus(const Status& status){_status = status;}

Priority& priority(){return _priority;}
const Priority& priority() const {return _priority;}
void setPriority(const Priority& priority){_priority = priority;}

Price& price(){return _price;}
const Price& price() const {return _price;}
void setPrice(const Price& price){_price = price;}

ml::Vec<std::string>& files(){return _files;}
const ml::Vec<std::string>& files() const {return _files;}


ml::Vec<Version>& version(){return _version;}
const ml::Vec<Version>& version() const {return _version;}


bool addFilesToCmds(){return _addFilesToCmds;}
const bool addFilesToCmds() const {return _addFilesToCmds;}
void setAddFilesToCmds(bool addFilesToCmds){_addFilesToCmds = addFilesToCmds;}

ml::Vec<std::function<void(const std::string& event)>>& onevents(){return _onevents;}
const ml::Vec<std::function<void(const std::string& event)>>& onevents() const {return _onevents;}
void setOnevents(const ml::Vec<std::function<void(const std::string& event)>>& onevents){_onevents = onevents;}

Tasks& subtasks(){return _subtasks;}
const Tasks& subtasks() const {return _subtasks;}


Tasks* parent(){return _parent;}
const Tasks* parent() const {return _parent;}
void setParent(Tasks* parent){_parent = parent;}

ml::Events& events(){return _events;}
const ml::Events& events() const {return _events;}


