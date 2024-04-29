#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WLineEdit.h>
#include <Wt/WSpinBox.h>
#include <Wt/WPushButton.h>
#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/backend/MySQL.h>
#include <Wt/WTable.h>
#include <iostream>
#include <string>
#include <fstream>

namespace dbo = Wt::Dbo;

class stocks;

typedef dbo::collection<typename dbo::ptr<stocks>> stocksCollection;

class stocks : public dbo::Dbo<stocks> {
public:
    std::string item;
    int stock;

    stocks() : stock(0) {}

    template<class Action>
    void persist(Action& a) {
        dbo::field(a, item, "item");
        dbo::field(a, stock, "stock");
    }

    typedef dbo::ptr<stocks> Ptr;
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
            auto mysql = std::make_unique<Wt::Dbo::backend::MySQL>("cafems", "root", "", "localhost");

            logfile << "Database connected successfully." << std::endl;

            // Create a session with the MySQL backend
            session.setConnection(std::move(mysql));

            // Start a transaction
            dbo::Transaction transaction(session);

            // Map User class to the database
            session.mapClass<stocks>("stocks"); // Mapping User class to a table named "user"

            // Retrieve data from the database
            stocksCollection itemstock = session.find<stocks>();

            // Display user information in a table
            auto table = std::make_unique<Wt::WTable>();

            // Add table headers
            table->elementAt(0, 0)->addWidget(std::make_unique<Wt::WText>("ITEM"));
            table->elementAt(0, 1)->addWidget(std::make_unique<Wt::WText>("STOCK"));

            // Add user information to the table
            int row = 1; // Start from row 1 to leave space for headers
            for (const auto& stock : itemstock) {
                table->elementAt(row, 0)->addWidget(std::make_unique<Wt::WText>(stock->item));
                table->elementAt(row, 1)->addWidget(std::make_unique<Wt::WText>(std::to_string(stock->stock)));
                ++row;
            }

            // Add table to root
            root()->addWidget(std::move(table));

            // Create UI elements with Bootstrap classes
            auto container = std::make_unique<Wt::WContainerWidget>();
            container->setStyleClass("container");

            auto inputName = std::make_unique<Wt::WLineEdit>();
            inputName->setStyleClass("form-control");
            auto inputNamePtr = inputName.get();
            container->addWidget(std::move(inputName));

            auto inputStock = std::make_unique<Wt::WSpinBox>();
            inputStock->setStyleClass("form-control");
            auto inputStockPtr = inputStock.get();
            container->addWidget(std::move(inputStock));

            auto buttonAddStock = std::make_unique<Wt::WPushButton>("Add Stock");
            buttonAddStock->setStyleClass("btn btn-primary");

            buttonAddStock->clicked().connect([this, inputNamePtr, inputStockPtr] {
                addStock(session, inputNamePtr->text().toUTF8(), inputStockPtr->value());
                refreshTable(); // Refresh table after adding
                });
            container->addWidget(std::move(buttonAddStock));

            auto buttonDeleteStock = std::make_unique<Wt::WPushButton>("Delete Stock");
            buttonDeleteStock->setStyleClass("btn btn-danger");

            buttonDeleteStock->clicked().connect([this, inputNamePtr] {
                deleteStock(session, inputNamePtr->text().toUTF8());
                refreshTable(); // Refresh table after deleting
                });
            container->addWidget(std::move(buttonDeleteStock));

            // Add container to root
            root()->addWidget(std::move(container));

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

private:
    dbo::Session session;

    void addStock(dbo::Session& session, const std::string& item, int stock) {
        dbo::Transaction transaction(session);
        auto newStock = std::make_unique<stocks>();
        newStock->item = item;
        newStock->stock = stock;
        session.add(std::move(newStock));
        transaction.commit();
    }

    void deleteStock(dbo::Session& session, const std::string& item) {
        dbo::Transaction transaction(session);
        // Find the item in the session
        dbo::ptr<stocks> stockPtr = session.find<stocks>().where("item = ?").bind(item);
        // If the item exists, remove it from the session
        if (stockPtr) {
            stockPtr.remove(); // Remove the object from the session
            std::cout << "Item removed from database: " << item << std::endl;
        }
        else {
            std::cout << "Item not found in database: " << item << std::endl;
        }
        transaction.commit();
    }

    void refreshTable() {
        // Clear current table
        root()->clear();

        // Refresh data and rebuild table
        dbo::Transaction transaction(session);
        stocksCollection itemstock = session.find<stocks>();

        auto table = std::make_unique<Wt::WTable>();

        // Add table headers
        table->elementAt(0, 0)->addWidget(std::make_unique<Wt::WText>("ITEM"));
        table->elementAt(0, 1)->addWidget(std::make_unique<Wt::WText>("STOCK"));

        // Add user information to the table
        int row = 1; // Start from row 1 to leave space for headers
        for (const auto& stock : itemstock) {
            table->elementAt(row, 0)->addWidget(std::make_unique<Wt::WText>(stock->item));
            table->elementAt(row, 1)->addWidget(std::make_unique<Wt::WText>(std::to_string(stock->stock)));
            ++row; 
        }

        // Add table to root
        root()->addWidget(std::move(table));

        transaction.commit();
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
