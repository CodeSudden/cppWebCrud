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
    std::string flavor;
    int stock;

    stocks() : stock(0) {}

    template<class Action>
    void persist(Action& a) {
        dbo::field(a, flavor, "flavor");
        dbo::field(a, stock, "stocks");
    }

    typedef dbo::ptr<stocks> Ptr;
};

class sales : public dbo::Dbo<sales> {
public:
    std::string flavor;
    int quantity;
    std::string date;


    sales() : quantity(0) {}

    template<class Action>
    void persist(Action& a) {
        dbo::field(a, flavor, "flavor");
        dbo::field(a, quantity, "quantity");
        dbo::field(a, date, "date");

    }

    typedef dbo::ptr<sales> Ptr;
};

class MyApplication : public Wt::WApplication {

private:
    static bool isLoggedIn; // Flag to track login status

public:
    MyApplication(const Wt::WEnvironment& env) : Wt::WApplication(env) {
        auto app = WApplication::instance();
        app->setTheme(std::make_shared<Wt::WBootstrap5Theme>());
        // Check if the user is already logged in

        setupdatabase();

        if (!isLoggedIn) {
            purchaseorder();
        }
        else {
            initializeMainApplication(); // Initialize main application if already logged in
        }
    }

private:
    void showLoginForm() {

        root()->clear();

        auto container = std::make_unique<Wt::WContainerWidget>();
        container->setStyleClass("container d-flex justify-content-center align-items-center vh-100");

        // Create a div to contain the login form
        auto loginForm = std::make_unique<Wt::WContainerWidget>();
        loginForm->setStyleClass("d-flex flex-column w-75");

        // Title form login
        loginForm->addNew<Wt::WText>("ServeBetes")->setStyleClass("fs-3 fw-bolder w-100 text-center");

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
                (username == "kevintusi@gmail.servebetes.com" && password == "12345") ||
                (username == "ivanjuanier@gmail.servebetes.com" && password == "12345") ||
                (username == "andreamagallanes@gmail.servebetes.com" && password == "12345") ||
                (username == "hristianumadac@gmail.servebets.com" && password == "12345")) {
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

        // go back button
        auto order = std::make_unique<Wt::WPushButton>("Go back");
        order->setStyleClass("btn btn-secondary mt-5 w-25 m-auto");
        order->clicked().connect(this, &MyApplication::purchaseorder); // Connect the clicked signal to showLoginForm()

        loginForm->addWidget(std::move(loginButton));
        loginForm->addWidget(std::move(order));
        container->addWidget(std::move(loginForm));

        // Add the container to the root
        root()->addWidget(std::move(container));
    }

    void purchaseorder() {

        root()->clear();

        auto storecontainer = std::make_unique<Wt::WContainerWidget>();
        storecontainer->setStyleClass("container d-flex flex-column align-items-center justify-content-center vh-100");

        // Create a div to contain the login form
        auto storeform = std::make_unique<Wt::WContainerWidget>();
        storeform->setStyleClass("d-flex flex-column w-75 align-items-center");

        // Title form login
        storeform->addNew<Wt::WText>("ServeBetes")->setStyleClass("fs-3 fw-bolder w-100 text-center");
        storeform->addNew<Wt::WText>("Order Your Favorite Ice cream")->setStyleClass("fs-5 fw-light w-100 text-center");


        // Flavor input
        auto inputflavor = std::make_unique<Wt::WLineEdit>();
        inputflavor->setPlaceholderText("Flavor");
        inputflavor->setStyleClass("mt-3 w-25");
        auto flavorPtr = inputflavor.get();
        storeform->addWidget(std::move(inputflavor));

        // Quantity input
        auto inputquantity = std::make_unique<Wt::WSpinBox>();
        inputquantity->setPlaceholderText("quantity");
        inputquantity->setStyleClass("mt-3 w-25");
        auto quantityPtr = inputquantity.get();
        storeform->addWidget(std::move(inputquantity));

        // Order button
        auto orderbutton = std::make_unique<Wt::WPushButton>("Order Item");
        orderbutton->setStyleClass("btn btn-success mt-3 w-25");

        orderbutton->clicked().connect([this, flavorPtr, quantityPtr] {
            // Get the current date and time
            auto now = std::chrono::system_clock::now();
            std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

            // Initialize a struct tm variable to store the local time
            std::tm localTime = {};
            // Call localtime_s to populate the localTime variable with the current local time
            localtime_s(&localTime, &currentTime);

            // Format the current date as YYYY-MM-DD
            char currentDate[11];
            strftime(currentDate, sizeof(currentDate), "%Y-%m-%d", &localTime);

            // Add order to the database with the current date
            addorder(session, flavorPtr->text().toUTF8(), quantityPtr->value(), currentDate);
            root()->doJavaScript("location.reload();");
            });


        // Create a login button
        auto login = std::make_unique<Wt::WPushButton>("Login as admin");
        login->setStyleClass("btn btn-primary mt-5");
        login->clicked().connect(this, &MyApplication::showLoginForm); // Connect the clicked signal to showLoginForm()

        storeform->addWidget(std::move(orderbutton));
        storeform->addWidget(std::move(login));

        storecontainer->addWidget(std::move(storeform));

        // Add the container to the root
        root()->addWidget(std::move(storecontainer));
    }



    void setupdatabase() {
        try {
            // Open logfile for writing
            std::ofstream logfile("logfile.txt", std::ios::app);
            if (!logfile.is_open()) {
                throw std::runtime_error("Failed to open logfile");
            }

            // Log messages
            logfile << "Connecting to database..." << std::endl;

            // Set up MySQL database connection
            auto mysql = std::make_unique<Wt::Dbo::backend::MySQL>("servebetes", "root", "", "localhost");

            logfile << "Database connected successfully." << std::endl;

            // Create a session with the MySQL backend
            session.setConnection(std::move(mysql));

            // Start a transaction
            dbo::Transaction transaction(session);

            // Map User class to the database
            session.mapClass<stocks>("stocks"); // Mapping User class to a table named "user"
            session.mapClass<sales>("sales");

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

    void initializeMainApplication() {

        // Start a transaction
        dbo::Transaction transaction(session);

        // Retrieve data from the database
        stocksCollection itemstock = session.find<stocks>();

        // Create UI elements with Bootstrap classes
        auto container = std::make_unique<Wt::WContainerWidget>();
        container->setStyleClass("container mb-5");

        auto flexContainer = std::make_unique<Wt::WContainerWidget>();
        flexContainer->setStyleClass("d-flex justify-content-between align-items-center");

        auto title = std::make_unique<Wt::WText>("ServeBetes Admin");
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
        itemCol->addWidget(std::make_unique<Wt::WText>("Flavor:"));
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
        auto buttonAddStock = std::make_unique<Wt::WPushButton>("Add Flavor");
        buttonAddStock->setStyleClass("btn btn-primary");
        buttonAddStock->clicked().connect([this, inputNamePtr, inputStockPtr] {
            addFlavor(session, inputNamePtr->text().toUTF8(), inputStockPtr->value());
            root()->doJavaScript("location.reload();");
            });
        buttonCol->addWidget(std::move(buttonAddStock));
        row1Container->addWidget(std::move(buttonCol));

        // Add the row container to the main container
        container->addWidget(std::move(row1Container));

        // Add container to root
        root()->addWidget(std::move(container));

        //widget
        auto tabWidget = std::make_unique<Wt::WTabWidget>();
        tabWidget->addStyleClass("container");

        // Create a container widget for the "Stocks" table
        auto stocksTable = std::make_unique<Wt::WTable>();
        auto* stocksTablePtr = stocksTable.get();

        // Add table headers for stocks
        stocksTable->elementAt(0, 0)->addWidget(std::make_unique<Wt::WText>("Flavor"))->setStyleClass("fs-6 fw-bold");
        stocksTable->elementAt(0, 1)->addWidget(std::make_unique<Wt::WText>("STOCK"))->setStyleClass("fs-6 fw-bold");
        stocksTable->elementAt(0, 2)->addWidget(std::make_unique<Wt::WText>("ACTION"))->setStyleClass("fs-6 fw-bold");
        stocksTable->elementAt(0, 3)->addWidget(std::make_unique<Wt::WText>("ADD STOCKS"))->setStyleClass("fs-6 fw-bold");

        // Add stock information to the table
        int stockRow = 1; // Start from row 1 to leave space for headers
        for (const auto& stock : itemstock) {
            stocksTable->elementAt(stockRow, 0)->addWidget(std::make_unique<Wt::WText>(stock->flavor));
            stocksTable->elementAt(stockRow, 1)->addWidget(std::make_unique<Wt::WText>(std::to_string(stock->stock)));

            // Create delete button for each stock row
            auto deleteButton = std::make_unique<Wt::WPushButton>("DELETE");
            deleteButton->setStyleClass("btn btn-danger");

            deleteButton->clicked().connect([this, stock]() {
                deleteStock(session, stock->flavor);
                Wt::WApplication::instance()->doJavaScript("location.reload();");
                });
            stocksTable->elementAt(stockRow, 2)->addWidget(std::move(deleteButton));

            auto inlineContainer = std::make_unique<Wt::WHBoxLayout>();

            auto inputNewStock = std::make_unique<Wt::WSpinBox>();
            inputNewStock->setStyleClass("form-control w-25");
            auto inputNewStockPtr = inputNewStock.get();
            inlineContainer->addWidget(std::move(inputNewStock));

            // Create update button for each stock row
            auto updateButton = std::make_unique<Wt::WPushButton>("Add Stock");
            updateButton->setStyleClass("btn btn-success");
            updateButton->clicked().connect([this, stock, inputNewStockPtr]() {
                addStock(session, stock->flavor, inputNewStockPtr->value());
                root()->doJavaScript("location.reload();");
                });
            inlineContainer->addWidget(std::move(updateButton));
            stocksTable->elementAt(stockRow, 3)->setLayout(std::move(inlineContainer));
            ++stockRow;
        }

        auto stocksTab = tabWidget->addTab(std::move(stocksTable), "Stocks");
        stocksTab->contents()->setStyleClass("table table-striped table-hover container"); // Use Bootstrap's container-fluid class for full width

// ******************************************************************************************************************************************************************* //

        auto reportContent = std::make_unique<Wt::WContainerWidget>();
        auto* reportContentPtr = reportContent.get();

        reportContent->addWidget(std::make_unique<Wt::WText>("Sales Analytics"))->setStyleClass("fs-4 fw-bold");
        reportContent->addNew<Wt::WBreak>();
        reportContent->addNew<Wt::WBreak>();
        reportContent->addWidget(std::make_unique<Wt::WText>("Most purchased flavors daily (10 days)"))->setStyleClass("fs-4 fw-bold");

        // Create a table
        auto stable = std::make_unique<Wt::WTable>();
        stable->addStyleClass("table table-striped table-hover");

        // Add table headers
        stable->elementAt(0, 0)->addWidget(std::make_unique<Wt::WText>("DATE"))->setStyleClass("fs-6 fw-bold");
        stable->elementAt(0, 1)->addWidget(std::make_unique<Wt::WText>("FLAVOR"))->setStyleClass("fs-6 fw-bold");
        stable->elementAt(0, 2)->addWidget(std::make_unique<Wt::WText>("QUANTITY"))->setStyleClass("fs-6 fw-bold");

        // Get the current date
        std::time_t currentTime = std::time(nullptr);
        std::tm currentDate;
        #if defined(_WIN32)
        localtime_s(&currentDate, &currentTime);
        #else
        currentDate = *std::localtime(&currentTime);
        #endif

        // Calculate the date 10 days ago
        std::tm tenDaysAgo = currentDate;
        tenDaysAgo.tm_mday -= 10;
        std::mktime(&tenDaysAgo);

        char formattedDate[11]; // Room for "YYYY-MM-DD\0"
        std::strftime(formattedDate, sizeof(formattedDate), "%Y-%m-%d", &tenDaysAgo);

        // Find all sales items within the last 10 days
        dbo::collection<sales::Ptr> allSales = session.find<sales>().where("date >= ?").bind(std::string(formattedDate));
        dbo::collection<sales::Ptr> mostsale = session.find<sales>();


        // Create a map to store daily purchases
        std::map<std::string, std::map<std::string, int>> dailyPurchases; // Key: Date, Value: Map of flavor and quantity

        // Aggregate sales data by date and flavor
        for (const auto& saleItem : allSales) {
            dailyPurchases[saleItem->date][saleItem->flavor] += saleItem->quantity;
        }

        // Find the most purchased flavor for each date
        std::map<std::string, std::pair<std::string, int>> mostPurchasedFlavor; // Key: Date, Value: (Flavor, Quantity)
        for (const auto& dailyPurchase : dailyPurchases) {
            const std::string& date = dailyPurchase.first;
            const auto& flavorQuantities = dailyPurchase.second;
            std::string mostFlavor;
            int maxQuantity = 0;
            for (const auto& flavorQuantity : flavorQuantities) {
                if (flavorQuantity.second > maxQuantity) {
                    mostFlavor = flavorQuantity.first;
                    maxQuantity = flavorQuantity.second;
                }
            }
            mostPurchasedFlavor[date] = std::make_pair(mostFlavor, maxQuantity);
        }

        // Display the most purchased flavor for each date
        int row = 1;
        for (const auto& dailyPurchase : mostPurchasedFlavor) {
            stable->elementAt(row, 0)->addWidget(std::make_unique<Wt::WText>(dailyPurchase.first)); // Date
            stable->elementAt(row, 1)->addWidget(std::make_unique<Wt::WText>(dailyPurchase.second.first)); // Most purchased flavor
            stable->elementAt(row, 2)->addWidget(std::make_unique<Wt::WText>(std::to_string(dailyPurchase.second.second))); // Quantity
            ++row;
        }

        // Add the table to the container
        reportContent->addWidget(std::move(stable));

        // Create a table
        auto otable = std::make_unique<Wt::WTable>();
        otable->addStyleClass("table table-striped table-hover");

        // Add table headers
        otable->elementAt(0, 0)->addWidget(std::make_unique<Wt::WText>("DATE"))->setStyleClass("fs-6 fw-bold");
        otable->elementAt(0, 1)->addWidget(std::make_unique<Wt::WText>("FLAVOR"))->setStyleClass("fs-6 fw-bold");
        otable->elementAt(0, 2)->addWidget(std::make_unique<Wt::WText>("QUANTITY"))->setStyleClass("fs-6 fw-bold");

        // Display the most purchased flavor for each date
        int orow = 1;
        for (const auto& sale : mostsale) {
            otable->elementAt(orow, 0)->addWidget(std::make_unique<Wt::WText>(sale->date)); // Date
            otable->elementAt(orow, 1)->addWidget(std::make_unique<Wt::WText>(sale->flavor)); // Most purchased flavor
            otable->elementAt(orow, 2)->addWidget(std::make_unique<Wt::WText>(std::to_string(sale->quantity))); // Quantity
            ++orow;
        }

        reportContent->addWidget(std::move(otable));

        auto reportTab = tabWidget->addTab(std::move(reportContent), "Reports");
        reportTab->contents()->setStyleClass("container text-center");

// ******************************************************************************************************************************************************************* //

        // Connect to the clicked signal of each tab
        stocksTab->clicked().connect([stocksTablePtr, reportContentPtr]() {
            stocksTablePtr->setHidden(false);
            reportContentPtr->setHidden(true);
            });

        reportTab->clicked().connect([stocksTablePtr, reportContentPtr]() {
            stocksTablePtr->setHidden(true);
            reportContentPtr->setHidden(false);
            });

        // Add both tables to the root
        root()->addWidget(std::move(tabWidget));

        // Use Bootstrap's grid system to set fixed width for tab titles
        stocksTab->setStyleClass("col-1"); // Set fixed width for tab title

        // Commit the transaction
        transaction.commit();
    }

private:
    dbo::Session session;

    void addFlavor(dbo::Session& session, const std::string& item, int stock) {
        dbo::Transaction transaction(session);
        auto newflavor = std::make_unique<stocks>();
        newflavor->flavor = item;
        newflavor->stock = stock;
        session.add(std::move(newflavor));
        transaction.commit();
    }

    void addorder(dbo::Session& session, const std::string& flavor, int quantity, const std::string& date) {
        dbo::Transaction transaction(session);
        auto newOrder = std::make_unique<sales>();
        newOrder->flavor = flavor;
        newOrder->quantity = quantity;
        newOrder->date = date;
        session.add(std::move(newOrder));
        transaction.commit();
    }

    void deleteStock(dbo::Session& session, const std::string& flavor) {
        dbo::Transaction transaction(session);
        // Find the item in the session
        dbo::ptr<stocks> stockPtr = session.find<stocks>().where("flavor = ?").bind(flavor);
        // If the item exists, remove it from the session
        if (stockPtr) {
            stockPtr.remove(); // Remove the object from the session
            std::cout << "Item removed from database: " << flavor << std::endl;
        }
        else {
            std::cout << "Item not found in database: " << flavor << std::endl;
        }
        transaction.commit();
    }

    void addStock(dbo::Session& session, const std::string& flavor, int newStock) {
        dbo::Transaction transaction(session);
        dbo::ptr<stocks> stockPtr = session.find<stocks>().where("flavor = ?").bind(flavor);
        if (stockPtr) {
            stockPtr.modify()->stock += newStock; // add stock
            std::cout << "Stock updated for item: " << flavor << std::endl;
        }
        else {
            std::cout << "Item not found in database: " << flavor << std::endl;
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