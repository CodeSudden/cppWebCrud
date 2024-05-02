#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WLineEdit.h>
#include <Wt/WSpinBox.h>
#include <Wt/WPushButton.h>
#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/backend/MySQL.h>
#include <Wt/WBootstrap5Theme.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WVBoxLayout.h>
#include <Wt/WMenuItem.h>
#include <Wt/WTabWidget.h>
#include <Wt/WTable.h>
#include <Wt/WText.h>
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

class Supplier : public dbo::Dbo<Supplier> {
public:
    std::string supplier;
    std::string category;
    std::string status;

    Supplier() {}

    template<class Action>
    void persist(Action& a) {
        dbo::field(a, supplier, "supplier");
        dbo::field(a, category, "category");
        dbo::field(a, status, "status");
    }

    typedef dbo::ptr<Supplier> Ptr;
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
            session.mapClass<Supplier>("supplier");

            // Retrieve data from the database
            stocksCollection itemstock = session.find<stocks>();
            dbo::collection<Supplier::Ptr> suppliers = session.find<Supplier>();

            // Create UI elements with Bootstrap classes
            auto container = std::make_unique<Wt::WContainerWidget>();
            container->setStyleClass("container mb-5");

            container->addNew<Wt::WText>("Cafe Management System")->setStyleClass("fs-1 fw-bolder");
            container->addNew<Wt::WBreak>();

            // Create a row container
            auto row1Container = std::make_unique<Wt::WContainerWidget>();
            row1Container->setStyleClass("row p-2");

            // Item input
            auto itemCol = std::make_unique<Wt::WContainerWidget>();
            itemCol->setStyleClass("col fw-bold");
            itemCol->addWidget(std::make_unique<Wt::WText>("Item:"));
            auto inputName = std::make_unique<Wt::WLineEdit>();
            inputName->setStyleClass("form-control");
            auto inputNamePtr = inputName.get();
            itemCol->addWidget(std::move(inputName));
            row1Container->addWidget(std::move(itemCol));

            // Stocks input
            auto stocksCol = std::make_unique<Wt::WContainerWidget>();
            stocksCol->setStyleClass("col fw-bold");
            stocksCol->addWidget(std::make_unique<Wt::WText>("Stocks:"));
            auto inputStock = std::make_unique<Wt::WSpinBox>();
            inputStock->setStyleClass("form-control");
            auto inputStockPtr = inputStock.get();
            stocksCol->addWidget(std::move(inputStock));
            row1Container->addWidget(std::move(stocksCol));

            // Add Item button
            auto buttonCol = std::make_unique<Wt::WContainerWidget>();
            buttonCol->setStyleClass("col-auto align-self-end"); // Use col-auto to make the column width auto-adjust
            auto buttonAddStock = std::make_unique<Wt::WPushButton>("Add Item");
            buttonAddStock->setStyleClass("btn btn-primary");
            buttonAddStock->clicked().connect([this, inputNamePtr, inputStockPtr] {
                addStock(session, inputNamePtr->text().toUTF8(), inputStockPtr->value());
                refreshTable(); // Refresh table after adding
                });
            buttonCol->addWidget(std::move(buttonAddStock));
            row1Container->addWidget(std::move(buttonCol));

            auto row2Container = std::make_unique<Wt::WContainerWidget>();
            row2Container->setStyleClass("row p-2");

            // supplier input
            auto suppCol = std::make_unique<Wt::WContainerWidget>();
            suppCol->setStyleClass("col fw-bold");
            suppCol->addWidget(std::make_unique<Wt::WText>("Supplier Name:"));
            auto inputSupp = std::make_unique<Wt::WLineEdit>();
            inputSupp->setStyleClass("form-control");
            auto inputSuppPtr = inputSupp.get();
            suppCol->addWidget(std::move(inputSupp));
            row2Container->addWidget(std::move(suppCol));

            // Category input
            auto cateCol = std::make_unique<Wt::WContainerWidget>();
            cateCol->setStyleClass("col fw-bold");
            cateCol->addWidget(std::make_unique<Wt::WText>("Category:"));
            auto inputCate = std::make_unique<Wt::WLineEdit>();
            inputCate->setStyleClass("form-control");
            auto inputCatePtr = inputCate.get();
            cateCol->addWidget(std::move(inputCate));
            row2Container->addWidget(std::move(cateCol));

            // Status input
            auto statCol = std::make_unique<Wt::WContainerWidget>();
            statCol->setStyleClass("col fw-bold");
            statCol->addWidget(std::make_unique<Wt::WText>("Status:"));
            auto inputStat = std::make_unique<Wt::WLineEdit>();
            inputStat->setStyleClass("form-control");
            auto inputStatPtr = inputStat.get();
            statCol->addWidget(std::move(inputStat));
            row2Container->addWidget(std::move(statCol));

            // Add Supplier button
            auto button2Col = std::make_unique<Wt::WContainerWidget>();
            button2Col->setStyleClass("col-auto align-self-end"); // Use col-auto to make the column width auto-adjust
            auto buttonAddSupplier = std::make_unique<Wt::WPushButton>("Add Supplier");
            buttonAddSupplier->setStyleClass("btn btn-primary");
            buttonAddSupplier->clicked().connect([this, inputSuppPtr, inputCatePtr, inputStatPtr] {
                addSupplier(session, inputSuppPtr->text().toUTF8(), inputCatePtr->text().toUTF8(), inputStatPtr->text().toUTF8());
                refreshTable(); // Refresh table after adding
                });
            button2Col->addWidget(std::move(buttonAddSupplier));

            row2Container->addWidget(std::move(button2Col));

            // Add the row container to the main container
            container->addWidget(std::move(row1Container));
            container->addWidget(std::move(row2Container));

            // Add container to root
            root()->addWidget(std::move(container));

            //widget
            auto tabWidget = std::make_unique<Wt::WTabWidget>();
            tabWidget->addStyleClass("container");

            // Create a container widget for the "Stocks" table
            auto stocksTable = std::make_unique<Wt::WTable>();
            stocksTable->addStyleClass("table container table-hover");
            auto* stocksTablePtr = stocksTable.get();

            // Add table headers for stocks
            stocksTable->elementAt(0, 0)->addWidget(std::make_unique<Wt::WText>("ITEM"))->setStyleClass("fs-6 fw-bold");
            stocksTable->elementAt(0, 1)->addWidget(std::make_unique<Wt::WText>("STOCK"))->setStyleClass("fs-6 fw-bold");
            stocksTable->elementAt(0, 2)->addWidget(std::make_unique<Wt::WText>("ACTION"))->setStyleClass("fs-6 fw-bold");
            stocksTable->elementAt(0, 3)->addWidget(std::make_unique<Wt::WText>("UPDATE STOCKS"))->setStyleClass("fs-6 fw-bold");

            // Add stock information to the table
            int stockRow = 1; // Start from row 1 to leave space for headers
            for (const auto& stock : itemstock) {
                stocksTable->elementAt(stockRow, 0)->addWidget(std::make_unique<Wt::WText>(stock->item));
                stocksTable->elementAt(stockRow, 1)->addWidget(std::make_unique<Wt::WText>(std::to_string(stock->stock)));

                // Create delete button for each stock row
                auto deleteButton = std::make_unique<Wt::WPushButton>("DELETE");
                deleteButton->setStyleClass("btn btn-danger");

                deleteButton->clicked().connect([this, stock]() {
                    deleteStock(session, stock->item);
                    refreshTable(); // Refresh table after deleting
                    });
                stocksTable->elementAt(stockRow, 2)->addWidget(std::move(deleteButton));

                auto inlineContainer = std::make_unique<Wt::WHBoxLayout>();

                auto inputNewStock = std::make_unique<Wt::WSpinBox>();
                inputNewStock->setStyleClass("form-control w-25");
                auto inputNewStockPtr = inputNewStock.get();
                inlineContainer->addWidget(std::move(inputNewStock));

                // Create update button for each stock row
                auto updateButton = std::make_unique<Wt::WPushButton>("UPDATE");
                updateButton->setStyleClass("btn btn-success");
                updateButton->clicked().connect([this, stock, inputNewStockPtr]() {
                    updateStock(session, stock->item, inputNewStockPtr->value());
                    refreshTable(); // Refresh table after updating
                    });
                inlineContainer->addWidget(std::move(updateButton));
                stocksTable->elementAt(stockRow, 3)->setLayout(std::move(inlineContainer));
                ++stockRow;
            }

            auto stocksTab = tabWidget->addTab(std::move(stocksTable), "Stocks");
            stocksTab->contents()->setStyleClass("container"); // Use Bootstrap's container-fluid class for full width

// ******************************************************************************************************************************************************************* //
            // Create a container widget for the "Suppliers" table
            auto suppliersTable = std::make_unique<Wt::WTable>();
            suppliersTable->addStyleClass("table container table-hover");
            auto* suppliersTablePtr = suppliersTable.get();

            // Add table headers for suppliers
            suppliersTable->elementAt(0, 0)->addWidget(std::make_unique<Wt::WText>("SUPPLIER"))->setStyleClass("fs-6 fw-bold");
            suppliersTable->elementAt(0, 1)->addWidget(std::make_unique<Wt::WText>("CATEGORY"))->setStyleClass("fs-6 fw-bold");
            suppliersTable->elementAt(0, 2)->addWidget(std::make_unique<Wt::WText>("STATUS"))->setStyleClass("fs-6 fw-bold");
            suppliersTable->elementAt(0, 3)->addWidget(std::make_unique<Wt::WText>("ACTIONS"))->setStyleClass("fs-6 fw-bold");

            // Add data from supplier table to the table
            int supplierRow = 1; // Start from row 1 to leave space for headers
            for (const auto& supplier : suppliers) {
                suppliersTable->elementAt(supplierRow, 0)->addWidget(std::make_unique<Wt::WText>(supplier->supplier));
                suppliersTable->elementAt(supplierRow, 1)->addWidget(std::make_unique<Wt::WText>(supplier->category));
                suppliersTable->elementAt(supplierRow, 2)->addWidget(std::make_unique<Wt::WText>(supplier->status));

                // Create update button for each supplier row
                auto update2Button = std::make_unique<Wt::WPushButton>("UPDATE");
                update2Button->setStyleClass("btn btn-success");
                update2Button->clicked().connect([this, supplier]() mutable {
                    // Create a modal dialog
                    auto modal = std::make_unique<Wt::WDialog>("Update Supplier");

                    // Add input fields to the dialog
                    auto newCategoryInput = modal->contents()->addWidget(std::make_unique<Wt::WLineEdit>());
                    auto newStatusInput = modal->contents()->addWidget(std::make_unique<Wt::WLineEdit>());

                    // Capture session by value
                    auto sessionCopy = session;

                    // Add a button to submit the input
                    auto submitButton = modal->footer()->addWidget(std::make_unique<Wt::WPushButton>("Submit"));
                    submitButton->clicked().connect([this, modal = std::move(modal), newCategoryInput, newStatusInput, supplier, sessionCopy]() mutable {
                        std::string newCategory = newCategoryInput->text().toUTF8();
                        std::string newStatus = newStatusInput->text().toUTF8();
                        updateSupplier(sessionCopy, supplier->supplier, newCategory, newStatus);
                        modal->accept(); // Close the modal after updating
                        refreshTable(); // Refresh table after updating
                        });

                    // Add a button to cancel the input
                    auto cancelButton = modal->footer()->addWidget(std::make_unique<Wt::WPushButton>("Cancel"));
                    cancelButton->clicked().connect([modal = std::move(modal)]() {
                        modal->reject(); // Close the modal without updating
                        });

                    // Show the modal
                    modal->show();
                    });

                suppliersTable->elementAt(supplierRow, 3)->addWidget(std::move(update2Button));

                auto delete2Button = std::make_unique<Wt::WPushButton>("DELETE");
                delete2Button->setStyleClass("btn btn-danger");
                delete2Button->clicked().connect([this, supplier]() {
                    deleteSupplier(session, supplier->supplier);
                    refreshTable(); // Refresh table after deleting
                    });
                suppliersTable->elementAt(supplierRow, 7)->addWidget(std::move(delete2Button));

                ++supplierRow;
            }

            auto suppliersTab = tabWidget->addTab(std::move(suppliersTable), "Suppliers");
            suppliersTab->contents()->setStyleClass("container"); // Use Bootstrap's container-fluid class for full width

            // Connect to the clicked signal of each tab
            stocksTab->clicked().connect([stocksTablePtr, suppliersTablePtr]() {
                stocksTablePtr->setHidden(false);
                suppliersTablePtr->setHidden(true);
                });

            suppliersTab->clicked().connect([stocksTablePtr, suppliersTablePtr]() {
                stocksTablePtr->setHidden(true);
                suppliersTablePtr->setHidden(false);
                });

            // Add both tables to the root
            root()->addWidget(std::move(tabWidget));

            // Use Bootstrap's grid system to set fixed width for tab titles
            stocksTab->setStyleClass("col-1"); // Set fixed width for tab title
            suppliersTab->setStyleClass("col-1"); // Set fixed width for tab title

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

    void addSupplier(dbo::Session& session, const std::string& supplier , const std::string& category, const std::string& status) {
        dbo::Transaction transaction(session);
        auto newSupplier = std::make_unique<Supplier>();
        newSupplier->supplier = supplier;
        newSupplier->category = category;
        newSupplier->status = status;
        session.add(std::move(newSupplier));
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

    void deleteSupplier(dbo::Session& session, const std::string& supplier) {
        dbo::Transaction transaction(session);
        // Find the supplier in the session
        dbo::ptr<Supplier> supplierPtr = session.find<Supplier>().where("supplier = ?").bind(supplier);
        // If the supplier exists, remove it from the session
        if (supplierPtr) {
            supplierPtr.remove(); // Remove the object from the session
            std::cout << "Item removed from database: " << supplier << std::endl;
        }
        else {
            std::cout << "Item not found in database: " << supplier << std::endl;
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

    void updateSupplier(dbo::Session& session, const std::string& supplierName,  const std::string& newCategory, const std::string& newStatus) {
        dbo::Transaction transaction(session);
        dbo::ptr<Supplier> supplierPtr = session.find<Supplier>().where("supplier = ?").bind(supplierName);
        if (supplierPtr) {
            //supplierPtr.modify()->supplier = newSupplier; // Update category const std::string& newSupplier,
            supplierPtr.modify()->category = newCategory; // Update category
            supplierPtr.modify()->status = newStatus; // Update status
            std::cout << "Supplier updated: " << supplierName << std::endl;
        }
        else {
            std::cout << "Supplier not found in database: " << supplierName << std::endl;
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