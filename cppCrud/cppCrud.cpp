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
    std::string item;
    std::string address;
    std::string contact;    


    Supplier() {}

    template<class Action>
    void persist(Action& a) {
        dbo::field(a, supplier, "supplier");
        dbo::field(a, item, "item");
        dbo::field(a, address, "address");
        dbo::field(a, contact, "contact");

    }

    typedef dbo::ptr<Supplier> Ptr;
};

class MyApplication : public Wt::WApplication {

private:
    static bool isLoggedIn; // Flag to track login status

public:
    MyApplication(const Wt::WEnvironment& env) : Wt::WApplication(env) {
        auto app = WApplication::instance();
        app->setTheme(std::make_shared<Wt::WBootstrap5Theme>());
        std::cout << "isLoggedIn: " << isLoggedIn << std::endl;
        // Check if the user is already logged in
        if (!isLoggedIn) {
            showLoginForm(); // Show login form if not logged in
        }
        else {
            initializeMainApplication(); // Initialize main application if already logged in
        }
    }

private:
    void showLoginForm() {
        auto container = std::make_unique<Wt::WContainerWidget>();
        container->setStyleClass("container d-flex justify-content-center align-items-center vh-100");

        // Create a div to contain the login form
        auto loginForm = std::make_unique<Wt::WContainerWidget>();
        loginForm->setStyleClass("d-flex flex-column w-75");

        // Title form login
        loginForm->addNew<Wt::WText>("Cafe Management System")->setStyleClass("fs-3 fw-bolder w-100 text-center");

        // Username input
        auto usernameEdit = std::make_unique<Wt::WLineEdit>();
        usernameEdit->setPlaceholderText("Username");
        usernameEdit->setStyleClass("mt-3 w-25 m-auto");
        auto unamePtr = usernameEdit.get();
        loginForm->addWidget(std::move(usernameEdit));

        // Password input
        auto passwordEdit = std::make_unique<Wt::WLineEdit>();
        passwordEdit->setPlaceholderText("Password");
        passwordEdit->setAttributeValue("type", "password");
        passwordEdit->setStyleClass("mt-3 w-25 m-auto");
        auto inputPassPtr = passwordEdit.get();
        loginForm->addWidget(std::move(passwordEdit));

        // Login button
        auto loginButton = std::make_unique<Wt::WPushButton>("Login");
        loginButton->setStyleClass("btn btn-primary mt-3 w-25 m-auto");

        // Handle login button click
        loginButton->clicked().connect([=] {
            std::string username = unamePtr->text().toUTF8();
            std::string password = inputPassPtr->text().toUTF8();

            // Verify username and password (dummy logic)
            if ((username == "a" && password == "a") ||
                (username == "NotSoGoodRaphael" && password == "iamsecondbest") ||
                (username == "Reyannty" && password == "ilovemybf")) {
                // Successful login, set isLoggedIn flag and initialize main application
                isLoggedIn = true;
                root()->clear(); // Clear login UI
                root()->doJavaScript("location.reload();");
            }
            else {
                // Display error message
                root()->addWidget(std::make_unique<Wt::WText>("Invalid username or password"));
            }

            });

        loginForm->addWidget(std::move(loginButton));
        container->addWidget(std::move(loginForm));

        // Add the container to the root
        root()->addWidget(std::move(container));
    }

    void initializeMainApplication() {
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

            auto flexContainer = std::make_unique<Wt::WContainerWidget>();
            flexContainer->setStyleClass("d-flex justify-content-between align-items-center");

            auto title = std::make_unique<Wt::WText>("Cafe Management System");
            title->setStyleClass("fs-1 fw-bolder");

            auto logoutButton = std::make_unique<Wt::WPushButton>("Logout");
            logoutButton->setStyleClass("btn btn-danger mt-3 w-25");
            logoutButton->clicked().connect([=] {
                isLoggedIn = false; // Reset isLoggedIn flag
                root()->clear(); // Clear main application UI
                showLoginForm(); // Show login form
                });

            flexContainer->addWidget(std::move(title));
            flexContainer->addWidget(std::move(logoutButton));

            container->addWidget(std::move(flexContainer));

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
                root()->doJavaScript("location.reload();");
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

            // item input
            auto supitemCol = std::make_unique<Wt::WContainerWidget>();
            supitemCol->setStyleClass("col fw-bold");
            supitemCol->addWidget(std::make_unique<Wt::WText>("Item:"));
            auto inputSupItem = std::make_unique<Wt::WLineEdit>();
            inputSupItem->setStyleClass("form-control");
            auto inputSupItemPtr = inputSupItem.get();
            supitemCol->addWidget(std::move(inputSupItem));
            row2Container->addWidget(std::move(supitemCol));

            // address input
            auto addCol = std::make_unique<Wt::WContainerWidget>();
            addCol->setStyleClass("col fw-bold");
            addCol->addWidget(std::make_unique<Wt::WText>("Address:"));
            auto inputAdd = std::make_unique<Wt::WLineEdit>();
            inputAdd->setStyleClass("form-control");
            auto inputAddPtr = inputAdd.get();
            addCol->addWidget(std::move(inputAdd));
            row2Container->addWidget(std::move(addCol));

            // contact input
            auto contCol = std::make_unique<Wt::WContainerWidget>();
            contCol->setStyleClass("col fw-bold");
            contCol->addWidget(std::make_unique<Wt::WText>("Contact:"));
            auto inputCont = std::make_unique<Wt::WLineEdit>();
            inputCont->setStyleClass("form-control");
            auto inputContPtr = inputCont.get();
            contCol->addWidget(std::move(inputCont));
            row2Container->addWidget(std::move(contCol));

            // Add Supplier button
            auto button2Col = std::make_unique<Wt::WContainerWidget>();
            button2Col->setStyleClass("col-auto align-self-end"); // Use col-auto to make the column width auto-adjust
            auto buttonAddSupplier = std::make_unique<Wt::WPushButton>("Add Supplier");
            buttonAddSupplier->setStyleClass("btn btn-primary");
            buttonAddSupplier->clicked().connect([this, inputSuppPtr, inputSupItemPtr, inputAddPtr, inputContPtr] {
                addSupplier(session, inputSuppPtr->text().toUTF8(), inputSupItemPtr->text().toUTF8(), inputAddPtr->text().toUTF8(), inputContPtr->text().toUTF8());
                Wt::WApplication::instance()->doJavaScript("location.reload();");
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
                    Wt::WApplication::instance()->doJavaScript("location.reload();");
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
                    root()->doJavaScript("location.reload();");
                    });
                inlineContainer->addWidget(std::move(updateButton));
                stocksTable->elementAt(stockRow, 3)->setLayout(std::move(inlineContainer));
                ++stockRow;
            }

            auto stocksTab = tabWidget->addTab(std::move(stocksTable), "Stocks");
            stocksTab->contents()->setStyleClass("table table-striped table-hover container"); // Use Bootstrap's container-fluid class for full width

// ******************************************************************************************************************************************************************* //
            // Create a container widget for the "Suppliers" table
            auto suppliersTable = std::make_unique<Wt::WTable>();
            auto* suppliersTablePtr = suppliersTable.get();

            // Add table headers for suppliers
            suppliersTable->elementAt(0, 0)->addWidget(std::make_unique<Wt::WText>("SUPPLIER"))->setStyleClass("fs-6 fw-bold");
            suppliersTable->elementAt(0, 1)->addWidget(std::make_unique<Wt::WText>("ITEM"))->setStyleClass("fs-6 fw-bold");
            suppliersTable->elementAt(0, 2)->addWidget(std::make_unique<Wt::WText>("ADDRESS"))->setStyleClass("fs-6 fw-bold");
            suppliersTable->elementAt(0, 3)->addWidget(std::make_unique<Wt::WText>("CONTACT"))->setStyleClass("fs-6 fw-bold");
            suppliersTable->elementAt(0, 4)->addWidget(std::make_unique<Wt::WText>("UPDATE SUPPLIER"))->setStyleClass("fs-6 fw-bold");


            // Add data from supplier table to the table
            int supplierRow = 1; // Start from row 1 to leave space for headers
            for (const auto& supplier : suppliers) {
                suppliersTable->elementAt(supplierRow, 0)->addWidget(std::make_unique<Wt::WText>(supplier->supplier));
                suppliersTable->elementAt(supplierRow, 1)->addWidget(std::make_unique<Wt::WText>(supplier->item));
                suppliersTable->elementAt(supplierRow, 2)->addWidget(std::make_unique<Wt::WText>(supplier->address));
                suppliersTable->elementAt(supplierRow, 3)->addWidget(std::make_unique<Wt::WText>(supplier->contact));

                // Create input fields for new category and status
                auto inputNewSupplier = std::make_unique<Wt::WLineEdit>();
                inputNewSupplier->setPlaceholderText("New Supplier Name");
                inputNewSupplier->setStyleClass("mb-1");

                auto inputNewItem = std::make_unique<Wt::WLineEdit>(); // Assuming category is text input
                inputNewItem->setPlaceholderText("New Item Name");
                inputNewItem->setStyleClass("mb-1");

                auto inputNewAddress = std::make_unique<Wt::WLineEdit>(); // Assuming status is text input
                inputNewAddress->setPlaceholderText("New Contact");
                inputNewAddress->setStyleClass("mb-1");

                auto inputNewContact = std::make_unique<Wt::WLineEdit>(); // Assuming status is text input
                inputNewContact->setPlaceholderText("New Status");
                inputNewContact->setStyleClass("mb-1");

                auto inputNewSuppplierPtr = inputNewSupplier.get();
                auto inputNewItemPtr = inputNewItem.get();
                auto inputNewAddressPtr = inputNewAddress.get();
                auto inputNewContactPtr = inputNewContact.get();

                suppliersTable->elementAt(supplierRow, 4)->addWidget(std::move(inputNewSupplier));
                suppliersTable->elementAt(supplierRow, 4)->addWidget(std::move(inputNewItem));
                suppliersTable->elementAt(supplierRow, 4)->addWidget(std::move(inputNewAddress));
                suppliersTable->elementAt(supplierRow, 4)->addWidget(std::move(inputNewContact));

                // Create update button for each supplier row
                auto update2Button = std::make_unique<Wt::WPushButton>("UPDATE");
                update2Button->setStyleClass("btn btn-success");
                update2Button->clicked().connect([this, supplier, inputNewSuppplierPtr, inputNewItemPtr, inputNewAddressPtr, inputNewContactPtr]() {
                    std::string newSupplier = inputNewSuppplierPtr->text().toUTF8();
                    std::string newCategory = inputNewItemPtr->text().toUTF8();
                    std::string newStatus = inputNewAddressPtr->text().toUTF8();
                    std::string newContact = inputNewContactPtr->text().toUTF8();
                    updateSupplier(session, supplier->supplier, newSupplier, newCategory, newStatus, newContact);
                    root()->doJavaScript("location.reload();");
                    });
                suppliersTable->elementAt(supplierRow, 5)->addWidget(std::move(update2Button));

                auto delete2Button = std::make_unique<Wt::WPushButton>("DELETE");
                delete2Button->setStyleClass("btn btn-danger");
                delete2Button->clicked().connect([this, supplier]() {
                    deleteSupplier(session, supplier->supplier);
                    root()->doJavaScript("location.reload();");
                    });
                suppliersTable->elementAt(supplierRow, 6)->addWidget(std::move(delete2Button));

                ++supplierRow;
            }

            auto suppliersTab = tabWidget->addTab(std::move(suppliersTable), "Suppliers");
            suppliersTab->contents()->setStyleClass("table table-hover table-striped  container"); // Use Bootstrap's container-fluid class for full width

// ******************************************************************************************************************************************************************* //

            auto reportContent = std::make_unique<Wt::WContainerWidget>();
            auto* reportContentPtr = reportContent.get();

            reportContent->addWidget(std::make_unique<Wt::WText>("Weekly Low stock"))->setStyleClass("fs-4 fw-bold");

            // Create a table
            auto stable = std::make_unique<Wt::WTable>();
            stable->addStyleClass("table table-striped table-hover");

            // Add table headers
            stable->elementAt(0, 0)->addWidget(std::make_unique<Wt::WText>("Week"))->setStyleClass("fs-6 fw-bold");
            stable->elementAt(0, 1)->addWidget(std::make_unique<Wt::WText>("Low Stock Item"))->setStyleClass("fs-6 fw-bold");
            stable->elementAt(0, 2)->addWidget(std::make_unique<Wt::WText>("Stocks"))->setStyleClass("fs-6 fw-bold");

            // Find all stocks
            stocksCollection lowStock = session.find<stocks>();

            // Initialize variables to store the minimum stock and corresponding item
            int minStock = std::numeric_limits<int>::max(); // Initialize with maximum possible value
            std::string minStockItem;

            // Iterate over each stock item to find the minimum stock
            for (const auto& stockItem : lowStock) {
                if (stockItem->stock < minStock) {
                    minStock = stockItem->stock;
                    minStockItem = stockItem->item;
                }
            }

            // Add the weekly lowest stock and its corresponding item to the table
            stable->elementAt(1, 0)->addWidget(std::make_unique<Wt::WText>("Week 4")); // Week
            stable->elementAt(1, 1)->addWidget(std::make_unique<Wt::WText>(minStockItem)); // MinStockItem
            stable->elementAt(1, 2)->addWidget(std::make_unique<Wt::WText>(std::to_string(minStock))); // MinStock

            // Add the table to the container
            reportContent->addWidget(std::move(stable));


            // Create a table
            reportContent->addWidget(std::make_unique<Wt::WText>("Weekly Stock"))->setStyleClass("fs-4 fw-bold");
            stocksCollection weekstock = session.find<stocks>();
            auto wtable = std::make_unique<Wt::WTable>();
            wtable->addStyleClass("table table-striped table-hover");

            // Add table headers
            wtable->elementAt(0, 0)->addWidget(std::make_unique<Wt::WText>("Week"))->setStyleClass("fs-6 fw-bold");
            wtable->elementAt(0, 1)->addWidget(std::make_unique<Wt::WText>("Item"))->setStyleClass("fs-6 fw-bold");
            wtable->elementAt(0, 2)->addWidget(std::make_unique<Wt::WText>("Stock"))->setStyleClass("fs-6 fw-bold");

            int wstockrow = 1;
            wtable->elementAt(1, 0)->addWidget(std::make_unique<Wt::WText>("Week 1"))->setStyleClass("fw-bold");
            for (const auto& wstock : weekstock) {
                wtable->elementAt(wstockrow, 1)->addWidget(std::make_unique<Wt::WText>(wstock->item));
                wtable->elementAt(wstockrow, 2)->addWidget(std::make_unique<Wt::WText>(std::to_string(wstock->stock - 1)));
                ++wstockrow;
            }
            wtable->elementAt(wstockrow, 0)->addWidget(std::make_unique<Wt::WText>("Week 2"))->setStyleClass("fw-bold");
            stocksCollection weekstock2 = session.find<stocks>();
            for (const auto& wstock2 : weekstock2) {
                wtable->elementAt(wstockrow, 1)->addWidget(std::make_unique<Wt::WText>(wstock2->item));
                wtable->elementAt(wstockrow, 2)->addWidget(std::make_unique<Wt::WText>(std::to_string(wstock2->stock - 2)));
                ++wstockrow;
            }
            wtable->elementAt(wstockrow, 0)->addWidget(std::make_unique<Wt::WText>("Week 3"))->setStyleClass("fw-bold");
            stocksCollection weekstock3 = session.find<stocks>();
            for (const auto& wstock3 : weekstock3) {
                wtable->elementAt(wstockrow, 1)->addWidget(std::make_unique<Wt::WText>(wstock3->item));
                wtable->elementAt(wstockrow, 2)->addWidget(std::make_unique<Wt::WText>(std::to_string(wstock3->stock - 1)));
                ++wstockrow;
            }
            wtable->elementAt(wstockrow, 0)->addWidget(std::make_unique<Wt::WText>("Week 4"))->setStyleClass("fw-bold");
            stocksCollection weekstock4 = session.find<stocks>();
            for (const auto& wstock4 : weekstock4) {
                wtable->elementAt(wstockrow, 1)->addWidget(std::make_unique<Wt::WText>(wstock4->item));
                wtable->elementAt(wstockrow, 2)->addWidget(std::make_unique<Wt::WText>(std::to_string(wstock4->stock)));
                ++wstockrow;
            }
            reportContent->addWidget(std::move(wtable));


            // Create a delivery table
            reportContent->addWidget(std::make_unique<Wt::WText>("Delivery Dates"))->setStyleClass("fs-4 fw-bold");
            auto dtable = std::make_unique<Wt::WTable>();
            dtable->addStyleClass("table table-striped table-hover");

            // Add table headers
            dtable->elementAt(0, 0)->addWidget(std::make_unique<Wt::WText>("Date"))->setStyleClass("fs-6 fw-bold");
            dtable->elementAt(0, 1)->addWidget(std::make_unique<Wt::WText>("Item"))->setStyleClass("fs-6 fw-bold");
            dtable->elementAt(0, 2)->addWidget(std::make_unique<Wt::WText>("Stocks"))->setStyleClass("fs-6 fw-bold");

            // Add delivery dates
            dtable->elementAt(1, 0)->addWidget(std::make_unique<Wt::WText>("05-05-2024"));
            dtable->elementAt(1, 1)->addWidget(std::make_unique<Wt::WText>("Sugar"));
            dtable->elementAt(1, 2)->addWidget(std::make_unique<Wt::WText>("15"));

            // Add the table to the container
            reportContent->addWidget(std::move(dtable));

            auto reportTab = tabWidget->addTab(std::move(reportContent), "Reports");
            reportTab->contents()->setStyleClass("container");

// ******************************************************************************************************************************************************************* //

            // Connect to the clicked signal of each tab
            stocksTab->clicked().connect([stocksTablePtr, suppliersTablePtr, reportContentPtr]() {
                stocksTablePtr->setHidden(false);
                suppliersTablePtr->setHidden(true);
                reportContentPtr->setHidden(true);
                });

            suppliersTab->clicked().connect([stocksTablePtr, suppliersTablePtr, reportContentPtr]() {
                stocksTablePtr->setHidden(true);
                suppliersTablePtr->setHidden(false);
                reportContentPtr->setHidden(true);
                });

            reportTab->clicked().connect([stocksTablePtr, suppliersTablePtr, reportContentPtr]() {
                stocksTablePtr->setHidden(true);
                suppliersTablePtr->setHidden(true);
                reportContentPtr->setHidden(false);
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

    void addSupplier(dbo::Session& session, const std::string& supplier , const std::string& item, const std::string& address, const std::string& contact) {
        dbo::Transaction transaction(session);
        auto newSupplier = std::make_unique<Supplier>();
        newSupplier->supplier = supplier;
        newSupplier->item = item;
        newSupplier->address = address;
        newSupplier->contact = contact;
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

    void updateSupplier(dbo::Session& session, const std::string& supplierName, const std::string& newSupplier,  const std::string& newItem, const std::string& newAddress, const std::string& newContact) {
        dbo::Transaction transaction(session);
        dbo::ptr<Supplier> supplierPtr = session.find<Supplier>().where("supplier = ?").bind(supplierName);
        if (supplierPtr) {
            supplierPtr.modify()->supplier = newSupplier; // Update category
            supplierPtr.modify()->item = newItem; // Update category
            supplierPtr.modify()->address = newAddress; // Update status
            supplierPtr.modify()->contact = newContact;
            std::cout << "Supplier updated: " << supplierName << std::endl;
        }
        else {
            std::cout << "Supplier not found in database: " << supplierName << std::endl;
        }
        transaction.commit();
    }
};

bool MyApplication::isLoggedIn = false;

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