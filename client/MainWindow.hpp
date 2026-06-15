#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QMessageBox>
#include <QInputDialog>

#include "BankClient.hpp"

class MainWindow : public QMainWindow 
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void handleLogin();
    void handleRegister();
    void showBalance();
    void handleWithdraw();
    void handleDeposit();
    void handleTransfer();
    void handleLogout();
    void handleShowDebts();
    void handleGetLoan();
    void handlePayLoan();

private:
    BankClient* bankClient;

    QStackedWidget* stackedWidget;
    QWidget* loginPage;
    QWidget* dashboardPage;

    QLineEdit* userEdit;
    QLineEdit* passEdit;
    QPushButton* loginBtn;
    QPushButton* registerBtn;

    QLabel* welcomeLabel;
    QPushButton* balanceBtn;
    QPushButton* withdrawBtn;
    QPushButton* depositBtn;
    QPushButton* transferBtn;
    QPushButton* debtsBtn;
    QPushButton* getLoanBtn;
    QPushButton* payLoanBtn;
    QPushButton* logoutBtn;

    void setupLoginPage();
    void setupDashboardPage();
};