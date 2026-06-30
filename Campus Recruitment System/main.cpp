/*
============================================================================
CAMPUS RECRUITMENT SYSTEM 
============================================================================
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <exception>
#include <stdexcept>
using namespace std;

// ============================ GLOBALS & HELPERS ============================
const string SFILE = "students.dat", CFILE = "companies.dat", JFILE = "jobs.dat";
const string AFILE = "applications.dat";

void writeStr(ofstream &out, const string &s)
{
    int l = s.size();
    out.write((char *)&l, sizeof(l));
    out.write(s.c_str(), l);
}
string readStr(ifstream &in)
{
    int l;
    in.read((char *)&l, sizeof(l));
    char *b = new char[l + 1];
    in.read(b, l);
    b[l] = 0;
    string s(b);
    delete[] b;
    return s;
}

// ============================ EXCEPTIONS ============================
class DuplicateApp : public exception
{
    const char *what() const noexcept override { return "Already applied!"; }
};
class FileError : public exception
{
    const char *what() const noexcept override { return "File error!"; }
};

// ============================ ABSTRACT USER ============================
class User
{
protected:
    int id;
    string name, password;

public:
    User() : id(0) {}
    User(int i, string n, string p) : id(i), name(n), password(p) {}
    virtual ~User() {}
    int getId() const { return id; }
    string getName() const { return name; }
    string getPassword() const { return password; }
    void setName(string n) { name = n; }
    void setPassword(string p) { password = p; }

    virtual void display() const = 0;
    virtual void serialize(ofstream &out) const

    {
        out.write((char *)&id, sizeof(id));
        writeStr(out, name);
        writeStr(out, password);
    }
    virtual void deserialize(ifstream &in)
    {
        in.read((char *)&id, sizeof(id));
        name = readStr(in);
        password = readStr(in);
    }
    bool operator==(const User &o) const { return id == o.id; }
};

// ============================ STUDENT ============================
class Student : public User
{
    private:
    string dept;
    float cgpa;
    vector<string> skills;
    bool placed;


public:
    static int nextId;
    

    Student() : User(), dept(""), cgpa(0), placed(false) { id = nextId++; }

    Student(string n, string p, string d, float c)
        : User(0, n, p), dept(d), cgpa(c), placed(false) { id = nextId++; }
    void setDept(string d) { dept = d; }
    void setCgpa(float c) { cgpa = c; }
    void setPlaced(bool b) { placed = b; }
    float getCgpa() const { return cgpa; }
    bool isPlaced() const { return placed; }
    string getDept() const { return dept; }
    vector<string> getSkills() const { return skills; }
    void addSkill(string s) { skills.push_back(s); }

    void display() const override
    {
        cout << "\n========== STUDENT PROFILE ==========\n";
        cout << "ID         : " << id << "\n";
        cout << "Name       : " << name << "\n";
        cout << "Department : " << dept << "\n";
        cout << "CGPA       : " << cgpa << "\n";
        cout << "Placed     : " << (placed ? "YES" : "NO") << "\n";
        cout << "Skills     : ";
        if (skills.empty())
            cout << "None";
        else
            for (auto &s : skills)
                cout << s << " ";
        cout << "\n=====================================\n";
    }

    void serialize(ofstream &out) const override
    {
        User::serialize(out);
        writeStr(out, dept);
        out.write((char *)&cgpa, sizeof(cgpa));
        int sz = skills.size();
        out.write((char *)&sz, sizeof(sz));
        for (auto &s : skills)
            writeStr(out, s);
        out.write((char *)&placed, sizeof(placed));
    }
    void deserialize(ifstream &in) override
    {
        User::deserialize(in);
        dept = readStr(in);
        in.read((char *)&cgpa, sizeof(cgpa));
        int sz;
        in.read((char *)&sz, sizeof(sz));
        skills.clear();
        for (int i = 0; i < sz; ++i)
            skills.push_back(readStr(in));
        in.read((char *)&placed, sizeof(placed));
        if (id >= nextId)
            nextId = id + 1;
    }
    friend ostream &operator<<(ostream &os, const Student &s)
    {
        os << "ID:" << s.id << " | " << s.name << " | " << s.dept;
        return os;
    }
    friend istream &operator>>(istream &is, Student &s)
    {
        cout << "Enter Name: ";
        is >> s.name;
        cout << "Enter Password: ";
        is >> s.password;
        cout << "Enter Department: ";
        is >> s.dept;
        cout << "Enter CGPA: ";
        is >> s.cgpa;
        return is;
    }

    bool operator==(const Student &o) const { 
        return id == o.id; }

};
int Student::nextId = 1001;

// ============================  COMPANY   ============================

class Company : public User
{

public:
    static int nextId;
    Company() : User() { id = nextId++; }
    Company(string n, string p) : User(0, n, p) { id = nextId++; }

    void display() const override
    {
        cout << "\n========== COMPANY PROFILE ==========\n";
        cout << "ID   : " << id << "\n";
        cout << "Name : " << name << "\n";
        cout << "=====================================\n";
    }

    void serialize(ofstream &out) const override
    {
        User::serialize(out);
    }
    void deserialize(ifstream &in) override
    {
        User::deserialize(in);
        if (id >= nextId)
            nextId = id + 1;
    }

    friend ostream &operator<<(ostream &os, const Company &c)
    {
        os << "ID:" << c.id << " | " << c.name;
        return os;
    }
    friend istream &operator>>(istream &is, Company &c)
    {
        cout << "Enter Company Name: ";
        is >> c.name;
        cout << "Enter Password: ";
        is >> c.password;
        return is;
    }
    bool operator==(const Company &o) const { return id == o.id; }
};
int Company::nextId = 2001;

// ============================ ADMIN ============================
class Admin : public User
{
public:
    Admin() : User(1, "admin", "admin123") {}
    void display() const override { cout << "Admin: " << name << endl; }
};

// ============================ JOB (NO CGPA) ============================
class Job
{
    int jobId, companyId;
    string title, desc, req;
    bool open;

public:
    static int nextId;
    Job() : jobId(0), companyId(0), title(""), desc(""), req(""), open(true) { jobId = nextId++; }
    Job(int cid, string t, string d, string r)
        : companyId(cid), title(t), desc(d), req(r), open(true)
    {
        jobId = nextId++;
    }
    int getJobId() const { return jobId; }
    int getCompanyId() const { return companyId; }
    string getTitle() const { return title; }
    string getDesc() const { return desc; }
    string getReq() const { return req; }
    bool isOpen() const { return open; }
    void setOpen(bool o) { open = o; }

    void serialize(ofstream &out) const
    {
        out.write((char *)&jobId, sizeof(jobId));
        out.write((char *)&companyId, sizeof(companyId));
        writeStr(out, title);
        writeStr(out, desc);
        writeStr(out, req);
        out.write((char *)&open, sizeof(open));
    }
    void deserialize(ifstream &in)
    {
        in.read((char *)&jobId, sizeof(jobId));
        in.read((char *)&companyId, sizeof(companyId));
        title = readStr(in);
        desc = readStr(in);
        req = readStr(in);
        in.read((char *)&open, sizeof(open));
        if (jobId >= nextId)
            nextId = jobId + 1;
    }
    friend ostream &operator<<(ostream &os, const Job &j)
    {
        os << "JobID:" << j.jobId << " | " << j.title << " | " << (j.open ? "Open" : "Closed");
        return os;
    }
    friend istream &operator>>(istream &is, Job &j)
    {
        cout << "Enter Title: ";
        is >> j.title;
        cout << "Enter Description: ";
        is.ignore();
        getline(is, j.desc);
        cout << "Enter Requirements: ";
        getline(is, j.req);
        j.open = true;
        return is;
    }
    bool operator==(const Job &o) const { return jobId == o.jobId; }
};
int Job::nextId = 3001;

// ============================ APPLICATION ============================
class Application
{
    int appId, studentId, jobId;
    string status;

public:
    static int nextId;
    Application() : appId(0), studentId(0), jobId(0), status("Applied") { appId = nextId++; }
    Application(int sid, int jid) : studentId(sid), jobId(jid), status("Applied") { appId = nextId++; }
    int getAppId() const { return appId; }
    int getStudentId() const { return studentId; }
    int getJobId() const { return jobId; }
    string getStatus() const { return status; }
    void setStatus(string s) { status = s; }
    void serialize(ofstream &out) const
    {
        out.write((char *)&appId, sizeof(appId));
        out.write((char *)&studentId, sizeof(studentId));
        out.write((char *)&jobId, sizeof(jobId));
        writeStr(out, status);
    }
    void deserialize(ifstream &in)
    {
        in.read((char *)&appId, sizeof(appId));
        in.read((char *)&studentId, sizeof(studentId));
        in.read((char *)&jobId, sizeof(jobId));
        status = readStr(in);
        if (appId >= nextId)
            nextId = appId + 1;
    }
    friend ostream &operator<<(ostream &os, const Application &a)
    {
        os << "AppID:" << a.appId << " | Student:" << a.studentId
           << " | Job:" << a.jobId << " | " << a.status;
        return os;
    }
};
int Application::nextId = 4001;

// ============================ TEMPLATE REPOSITORY ============================

template <class T>
class Repository
{

public:
    static void save(const string &f, const vector<T> &data)
    {
        ofstream out(f, ios::binary);
        if (!out)

            throw FileError();

        int sz = data.size();
        out.write((char *)&sz, sizeof(sz));
        for (auto &item : data)
            item.serialize(out);
    }
    static void load(const string &f, vector<T> &data)
    {
        ifstream in(f, ios::binary);
        if (!in)
        {
            data.clear();
            return;
        }
        int sz;
        in.read((char *)&sz, sizeof(sz));
        if (!in.good())
        {
            data.clear();
            return;
        }
        data.clear();
        for (int i = 0; i < sz; ++i)
        {
            T item;

            item.deserialize(in);

            data.push_back(item);

        }
    }
};

// ============================ RECRUITMENT SYSTEM ============================
class RecruitmentSystem
{
    vector<Student> students;
    vector<Company> companies;
    vector<Job> jobs;
    vector<Application> apps;
    Admin admin;

    template <class T>
    T *find(vector<T> &v, int id)
    {
        auto it = find_if(v.begin(), v.end(), [id](T &obj)
                          { return obj.getId() == id; });
        return (it != v.end()) ? &(*it) : nullptr;
    }

public:
    RecruitmentSystem() { loadAll(); }

    ~RecruitmentSystem() { 
        saveAll(); 
    }

    void loadAll()
    {
        Repository<Student>::load(SFILE, students);
        Repository<Company>::load(CFILE, companies);
        Repository<Job>::load(JFILE, jobs);
        Repository<Application>::load(AFILE, apps);
    }
    void saveAll()
    {
        Repository<Student>::save(SFILE, students);
        Repository<Company>::save(CFILE, companies);
        Repository<Job>::save(JFILE, jobs);
        Repository<Application>::save(AFILE, apps);
    }

    Student *studentLogin(int id, string pass)
    {
        for (auto &s : students)
            if (s.getId() == id && s.getPassword() == pass)
                return &s;
        return nullptr;
    }
    Company *companyLogin(int id, string pass)
    {
        for (auto &c : companies)
            if (c.getId() == id && c.getPassword() == pass)
                return &c;
        return nullptr;
    }
    bool adminLogin(string pass) { return admin.getPassword() == pass; }

    Student *getStudentById(int id) { return find(students, id); }
    Job *findJob(int id)
    {
        auto it = find_if(jobs.begin(), jobs.end(), [id](Job &j)
                          { return j.getJobId() == id; });
        return (it != jobs.end()) ? &(*it) : nullptr;
    }

    void registerStudent(Student &s)
    {
        students.push_back(s);
        saveAll();
    }
    void registerCompany(Company &c)
    {
        companies.push_back(c);
        saveAll();
    }
    void postJob(Job &j)
    {
        jobs.push_back(j);
        saveAll();
    }

    void deleteStudent(int id)
    {
        auto it = remove_if(students.begin(), students.end(), [id](Student &s)
                            { return s.getId() == id; });
        if (it != students.end())
        {
            students.erase(it, students.end());
            apps.erase(remove_if(apps.begin(), apps.end(), [id](Application &a)
                                 { return a.getStudentId() == id; }),
                       apps.end());
            saveAll();
            cout << "Student deleted.\n";
        }
        else
            cout << "Not found.\n";
    }
    void deleteCompany(int id)
    {
        auto it = remove_if(companies.begin(), companies.end(), [id](Company &c)
                            { return c.getId() == id; });
        if (it != companies.end())
        {
            companies.erase(it, companies.end());
            jobs.erase(remove_if(jobs.begin(), jobs.end(), [id](Job &j)
                                 { return j.getCompanyId() == id; }),
                       jobs.end());
            saveAll();
            cout << "Company deleted.\n";
        }
        else
            cout << "Not found.\n";
    }
    void deleteJob(int id)
    {
        auto it = remove_if(jobs.begin(), jobs.end(), [id](Job &j)
                            { return j.getJobId() == id; });
        if (it != jobs.end())
        {
            jobs.erase(it, jobs.end());
            saveAll();
            cout << "Job deleted.\n";
        }
        else
            cout << "Not found.\n";
    }

    void showAllStudents()
    {
        for (auto &s : students)
            cout << s << endl;
    }
    void showAllCompanies()
    {
        for (auto &c : companies)
            cout << c << endl;
    }
    void showAllJobs()
    {
        for (auto &j : jobs)
            cout << j << endl;
    }

    vector<Job> getOpenJobs()
    {
        vector<Job> v;
        for (auto &j : jobs)
            if (j.isOpen())
                v.push_back(j);
        return v;
    }
    vector<Job> getCompanyJobs(int cid)
    {
        vector<Job> v;
        for (auto &j : jobs)
            if (j.getCompanyId() == cid)
                v.push_back(j);
        return v;
    }

    void applyJob(int sid, int jid)
    {
        for (auto &a : apps)
            if (a.getStudentId() == sid && a.getJobId() == jid)
                throw DuplicateApp();
        Student *s = find(students, sid);
        if (s && s->isPlaced())
            throw runtime_error("Already placed!");
        Job *j = findJob(jid);
        if (!j || !j->isOpen())
            throw runtime_error("Job unavailable!");
        apps.push_back(Application(sid, jid));
        saveAll();
        cout << "Applied! AppID: " << apps.back().getAppId() << endl;
    }

    vector<Application> getStudentApps(int sid)
    {
        vector<Application> v;
        for (auto &a : apps)
            if (a.getStudentId() == sid)
                v.push_back(a);
        return v;
    }
    vector<Application> getJobApps(int jid)
    {
        vector<Application> v;
        for (auto &a : apps)
            if (a.getJobId() == jid)
                v.push_back(a);
        return v;
    }

    void hireStudent(int appId)
    {
        for (auto &a : apps)
        {
            if (a.getAppId() == appId)
            {
                Student *s = find(students, a.getStudentId());
                if (!s)
                    throw runtime_error("Student not found!");
                if (s->isPlaced())
                    throw runtime_error("Student already placed!");
                Job *j = findJob(a.getJobId());
                if (!j)
                    throw runtime_error("Job not found!");
                a.setStatus("Hired");
                s->setPlaced(true);
                j->setOpen(false);
                saveAll();
                cout << " Student hired successfully!\n";
                return;
            }
        }
        throw runtime_error("Application not found!");
    }

    friend void generateReport(RecruitmentSystem &sys);
};

// ============================ FRIEND FUNCTION ============================
void generateReport(RecruitmentSystem &sys)
{
    int placed = 0;
    for (auto &s : sys.students)
        if (s.isPlaced())
            ++placed;
    cout << "\n========== SYSTEM REPORT ==========\n";
    cout << "Total Students    : " << sys.students.size() << "\n";
    cout << "Total Companies   : " << sys.companies.size() << "\n";
    cout << "Total Jobs        : " << sys.jobs.size() << "\n";
    cout << "Total Applications: " << sys.apps.size() << "\n";
    cout << "Placed Students   : " << placed << "\n";
    if (!sys.students.empty())
        cout << "Placement %       : " << (float)placed / sys.students.size() * 100 << "%\n";
    cout << "====================================\n";
}

// ============================ MENUS ============================
void mainMenu()
{
    cout << "\n========================================\n";
    cout << "   CAMPUS RECRUITMENT SYSTEM\n";
    cout << "========================================\n";
    cout << " 1. Admin Login\n";
    cout << " 2. Company Register\n";
    cout << " 3. Company Login\n";
    cout << " 4. Student Register\n";
    cout << " 5. Student Login\n";
    cout << " 6. Exit\n";
    cout << "========================================\n";
    cout << "Enter choice: ";
}

void stuMenu()
{
    cout << "\n========== STUDENT DASHBOARD ==========\n";
    cout << " 1. View Profile\n";
    cout << " 2. Upload Skills\n";
    cout << " 3. Edit Details\n";
    cout << " 4. View Jobs\n";
    cout << " 5. Apply for Job\n";
    cout << " 6. View My Applications\n";
    cout << " 7. Logout\n";
    cout << "=======================================\n";
    cout << "Enter choice: ";
}

void compMenu()
{
    cout << "\n========== COMPANY DASHBOARD ==========\n";
    cout << " 1. View Profile\n";
    cout << " 2. Edit Profile\n";
    cout << " 3. Post New Job\n";
    cout << " 4. Delete Job\n";
    cout << " 5. View My Jobs\n";
    cout << " 6. View Applicants for a Job\n";
    cout << " 7. Hire Applicant\n";
    cout << " 8. Logout\n";
    cout << "=======================================\n";
    cout << "Enter choice: ";
}

void adminMenu()
{
    cout << "\n=========== ADMIN PANEL ============\n";
    cout << " 1. View All Students\n";
    cout << " 2. View All Companies\n";
    cout << " 3. View All Jobs\n";
    cout << " 4. Delete Student\n";
    cout << " 5. Delete Company\n";
    cout << " 6. Delete Job\n";
    cout << " 7. Generate Report\n";
    cout << " 8. Logout\n";
    cout << "====================================\n";
    cout << "Enter choice: ";
}

// ============================ MAIN ============================
int main()
{
    RecruitmentSystem sys;
    int choice;
    while (true)
    {
        mainMenu();
        cin >> choice;
        if (choice == 6)
            break;
        try
        {
            // ---------- ADMIN ----------
            if (choice == 1)
            {
                string p;
                cout << "Enter Admin Password: ";
                cin >> p;
                if (!sys.adminLogin(p))
                {
                    cout << "Wrong password!\n";
                    continue;
                }
                int ac;
                while (true)
                {
                    adminMenu();
                    cin >> ac;
                    if (ac == 8)
                        break;
                    if (ac == 1)
                        sys.showAllStudents();
                    else if (ac == 2)
                        sys.showAllCompanies();
                    else if (ac == 3)
                        sys.showAllJobs();
                    else if (ac == 4)
                    {
                        int id;
                        cout << "Student ID: ";
                        cin >> id;
                        sys.deleteStudent(id);
                    }
                    else if (ac == 5)
                    {
                        int id;
                        cout << "Company ID: ";
                        cin >> id;
                        sys.deleteCompany(id);
                    }
                    else if (ac == 6)
                    {
                        int id;
                        cout << "Job ID: ";
                        cin >> id;
                        sys.deleteJob(id);
                    }
                    else if (ac == 7)
                        generateReport(sys);
                    else
                        cout << "Invalid option.\n";
                }
            }
            // ---------- COMPANY REGISTER ----------
            else if (choice == 2)
            {
                Company c;
                cin >> c;
                sys.registerCompany(c);
                cout << "\n Company Registered! ID: " << c.getId() << endl;
            }
            // ---------- COMPANY LOGIN ----------
            else if (choice == 3)
            {
                int id;
                string p;
                cout << "Company ID: ";
                cin >> id;
                cout << "Password: ";
                cin >> p;
                Company *c = sys.companyLogin(id, p);
                if (!c)
                {
                    cout << "Invalid credentials!\n";
                    continue;
                }
                cout << "\n Welcome " << c->getName() << "!\n";
                int cc;
                while (true)
                {
                    compMenu();
                    cin >> cc;
                    if (cc == 8)
                        break;

                    if (cc == 1)

                        c->display();

                    else if (cc == 2)
                    {
                        string n, p2;
                        cout << "New Name: ";
                        cin >> n;
                        cout << "New Pass: ";
                        cin >> p2;
                        c->setName(n);
                        c->setPassword(p2);
                        sys.saveAll();
                        cout << " Profile updated.\n";
                    }
                    else if (cc == 3)
                    {
                        Job j;
                        cin >> j;
                        Job j2(c->getId(), j.getTitle(), j.getDesc(), j.getReq());
                        sys.postJob(j2);
                        cout << " Job Posted! ID: " << j2.getJobId() << endl;
                    }
                    else if (cc == 4)
                    {
                        int jid;
                        cout << "Job ID to delete: ";
                        cin >> jid;
                        Job *jPtr = sys.findJob(jid);
                        if (jPtr && jPtr->getCompanyId() == c->getId())
                            sys.deleteJob(jid);
                        else
                            cout << "Not found or not yours.\n";
                    }
                    else if (cc == 5)
                    {
                        auto v = sys.getCompanyJobs(c->getId());
                        if (v.empty())
                            cout << "No jobs posted.\n";
                        else
                            for (auto &j : v)
                                cout << j << endl;
                    }
                    else if (cc == 6)
                    {
                        int jid;
                        cout << "Job ID: ";
                        cin >> jid;
                        auto v = sys.getJobApps(jid);
                        if (v.empty())
                            cout << "No applicants.\n";
                        else
                            for (auto &a : v)
                            {
                                Student *s = sys.getStudentById(a.getStudentId());
                                if (s)
                                    cout << "App:" << a.getAppId() << " | " << s->getName()
                                         << " | " << a.getStatus() << endl;
                            }
                    }
                    else if (cc == 7)
                    {
                        int aid;
                        cout << "Application ID to hire: ";
                        cin >> aid;
                        sys.hireStudent(aid);
                    }
                    else
                        cout << "Invalid option.\n";
                }
            }
            // ---------- STUDENT REGISTER ----------
            else if (choice == 4)
            {
                Student s;
                cin >> s;
                sys.registerStudent(s);
                cout << "\n Student Registered! ID: " << s.getId() << endl;
            }
            // ---------- STUDENT LOGIN ----------
            else if (choice == 5)
            {
                int id;
                string p;
                cout << "Student ID: ";
                cin >> id;
                cout << "Password: ";
                cin >> p;
                Student *s = sys.studentLogin(id, p);
                if (!s)
                {
                    cout << "Invalid credentials!\n";
                    continue;
                }
                cout << "\n Welcome " << s->getName() << "!\n";
                int sc;
                while (true)
                {
                    stuMenu();
                    cin >> sc;
                    if (sc == 7)
                        break;
                    if (sc == 1)
                        s->display();
                    else if (sc == 2)
                    {
                        cout << "Current Skills: ";
                        auto sk = s->getSkills();
                        if (sk.empty())
                            cout << "None";
                        else
                            for (auto &skl : sk)
                                cout << skl << " ";
                        cout << "\nEnter new skills (type 'done' to stop):\n";
                        string skill;
                        while (true)
                        {
                            cout << "Skill: ";
                            cin >> skill;
                            if (skill == "done")
                                break;
                            s->addSkill(skill);
                        }
                        sys.saveAll();
                        cout << " Skills updated.\n";
                    }
                    else if (sc == 3)
                    {
                        string n, p, d;
                        float c;
                        cout << "New Name: ";
                        cin >> n;
                        cout << "New Pass: ";
                        cin >> p;
                        cout << "New Dept: ";
                        cin >> d;
                        cout << "New CGPA: ";
                        cin >> c;
                        s->setName(n);
                        s->setPassword(p);
                        s->setDept(d);
                        s->setCgpa(c);
                        sys.saveAll();
                        cout << "✅ Profile updated.\n";
                    }
                    else if (sc == 4)
                    {
                        auto v = sys.getOpenJobs();
                        if (v.empty())
                            cout << "No open jobs.\n";
                        else
                            for (auto &j : v)
                                cout << j << "\nDescription: " << j.getDesc() << "\n\n";
                    }
                    else if (sc == 5)
                    {
                        int jid;
                        cout << "Job ID: ";
                        cin >> jid;
                        sys.applyJob(s->getId(), jid);
                    }
                    else if (sc == 6)
                    {
                        auto v = sys.getStudentApps(s->getId());
                        if (v.empty())
                            cout << "No applications.\n";
                        else
                            for (auto &a : v)
                                cout << a << endl;
                    }
                    else
                        cout << "Invalid option.\n";
                }
            }
            else
                cout << "Invalid option.\n";
        }
        catch (exception &e)
        {
            cout << " Error: " << e.what() << endl;
        }
    }
    cout << "\nExiting. All data saved.\n";
    return 0;
}