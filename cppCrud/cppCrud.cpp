#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WLineEdit.h>
#include <Wt/WSpinBox.h>
#include <Wt/WPushButton.h>
#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/backend/MySQL.h>
#include <Wt/WBootstrap5Theme.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WTable.h>
#include <Wt/WDialog.h>
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
        auto app = WApplication::instance();
        app->setTheme(std::make_shared<Wt::WBootstrap5Theme>());
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

            // Create UI elements with Bootstrap classes
            auto container = std::make_unique<Wt::WContainerWidget>();
            container->setStyleClass("container");

            container->addNew<Wt::WText>("Cafe Management System")->setStyleClass("fs-1 fw-bold");
            container->addNew<Wt::WBreak>();

            container->addNew<Wt::WText>("Item:");
            auto inputName = std::make_unique<Wt::WLineEdit>();
            inputName->setStyleClass("form-control");
            auto inputNamePtr = inputName.get();
            container->addWidget(std::move(inputName));

            container->addNew<Wt::WText>("Stocks:");
            auto inputStock = std::make_unique<Wt::WSpinBox>();
            inputStock->setStyleClass("form-control");
            auto inputStockPtr = inputStock.get();
            container->addWidget(std::move(inputStock));

            auto buttonAddStock = std::make_unique<Wt::WPushButton>("Add Item");
            buttonAddStock->setStyleClass("btn btn-primary");

            buttonAddStock->clicked().connect([this, inputNamePtr, inputStockPtr] {
                addStock(session, inputNamePtr->text().toUTF8(), inputStockPtr->value());
                refreshTable(); // Refresh table after adding
                });
            container->addWidget(std::move(buttonAddStock));

            // Add container to root
            root()->addWidget(std::move(container));

            // Display user information in a table
            auto table = std::make_unique<Wt::WTable>();
            table->addStyleClass("table container table-hover");

            // Add table headers
            table->elementAt(0, 0)->addWidget(std::make_unique<Wt::WText>("ITEM"));
            table->elementAt(0, 1)->addWidget(std::make_unique<Wt::WText>("STOCK"));
            table->elementAt(0, 2)->addWidget(std::make_unique<Wt::WText>("ACTION"));
            table->elementAt(0, 3)->addWidget(std::make_unique<Wt::WText>("UPDATE STOCKS"));

            // Add user information to the table
            int row = 1; // Start from row 1 to leave space for headers
            for (const auto& stock : itemstock) {
                table->elementAt(row, 0)->addWidget(std::make_unique<Wt::WText>(stock->item));
                table->elementAt(row, 1)->addWidget(std::make_unique<Wt::WText>(std::to_string(stock->stock)));

                // Create delete button for each row
                auto deleteButton = std::make_unique<Wt::WPushButton>("DELETE");
                deleteButton->setStyleClass("btn btn-danger");
                deleteButton->clicked().connect([this, stock]() {
                    deleteStock(session, stock->item);
                    refreshTable(); // Refresh table after deleting
                    });
                table->elementAt(row, 2)->addWidget(std::move(deleteButton));

                // Create a horizontal layout container
                auto inlineContainer = std::make_unique<Wt::WHBoxLayout>();

                // Create input field for new stock quantity
                auto inputNewStock = std::make_unique<Wt::WSpinBox>();
                inputNewStock->setStyleClass("form-control w-25");
                inlineContainer->addWidget(std::move(inputNewStock));

                // Create update button for each row
                auto updateButton = std::make_unique<Wt::WPushButton>("UPDATE");
                updateButton->setStyleClass("btn btn-success w-25");
                updateButton->clicked().connect([this, stock, inputNewStock = inputNewStock.get()]() {
                    updateStock(session, stock->item, inputNewStock->value());
                    refreshTable(); // Refresh table after updating
                    });
                inlineContainer->addWidget(std::move(updateButton));

                // Add the inline container to the table cell
                table->elementAt(row, 3)->setLayout(std::move(inlineContainer));

                ++row;
            }


            // Add table to root

            root()->addWidget(std::move(table));

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

    void updateStock(dbo::Session& session, const std::string& item, int newStock) {
        dbo::Transaction transaction(session);
        dbo::ptr<stocks> stockPtr = session.find<stocks>().where("item = ?").bind(item);
        if (stockPtr) {
            stockPtr.modify()->stock = newStock; // Update the stock quantity
            std::cout << "Stock updated for item: " << item << std::endl;
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