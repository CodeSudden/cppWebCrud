#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WText.h>
#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/backend/MySQL.h>
#include <iostream>
#include <string>
#include <fstream>

namespace dbo = Wt::Dbo;

class User;

typedef dbo::collection<typename dbo::ptr<User>> UsersCollection;

class User : public dbo::Dbo<User> {
public:
    std::string name;
    int age;

    User() : age(0) {}

    template<class Action>
    void persist(Action& a) {
        dbo::field(a, name, "name");
        dbo::field(a, age, "age");
    }

    typedef dbo::ptr<User> Ptr;
};

class MyApplication : public Wt::WApplication {
public:
    MyApplication(const Wt::WEnvironment& env) : Wt::WApplication(env) {
        try {
            // Open logfile for writing
            std::ofstream logfile("logfile.txt", std::ios::app);
            if (!logfile.is_open()) {
                throw std::runtime_error("Failed to open logfile");
            }

            // Log messages
            logfile << "Connecting to database..." << std::endl;

            // Set up MySQL database connection
            auto mysql = std::make_unique<Wt::Dbo::backend::MySQL>("user", "root", "", "localhost");

            logfile << "Database connected successfully." << std::endl;

            // Create a session with the MySQL backend
            dbo::Session session;
            session.setConnection(std::move(mysql));

            // Start a transaction
            dbo::Transaction transaction(session);

            // Map User class to the database
            session.mapClass<User>("user"); // Mapping User class to a table named "user"

            // Retrieve data from the database
            UsersCollection users = session.find<User>();

            // Display user information
            std::string allUsersInfo;
            for (const auto& user : users) {
                allUsersInfo += user->name + " - " + std::to_string(user->age) + "<br>";
            }
            root()->addWidget(std::make_unique<Wt::WText>(Wt::WString(allUsersInfo)));

            // Commit the transaction
            transaction.commit();

            logfile.close();
        }
        catch (const Wt::Dbo::Exception& e) { // Catch Wt Dbo exceptions
            // Log any Wt Dbo exceptions
            std::ofstream errorlog("errorlog.txt", std::ios::app);
            if (errorlog.is_open()) {
                errorlog << "Wt Dbo Exception: " << e.what() << std::endl;
                errorlog.close();
            }
        }
        catch (const std::exception& e) { // Catch other exceptions
            // Log any other exceptions
            std::ofstream errorlog("errorlog.txt", std::ios::app);
            if (errorlog.is_open()) {
                errorlog << "Exception: " << e.what() << std::endl;
                errorlog.close();
            }
        }
    }
};

int main(int argc, char** argv) {
    std::cout << "Connecting to database..." << std::endl;
    try {
        return Wt::WRun(argc, argv, [](const Wt::WEnvironment& env) {
            return std::make_unique<MyApplication>(env);
            });
    }
    catch (const std::exception& e) { // Catch exceptions from main
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
}
