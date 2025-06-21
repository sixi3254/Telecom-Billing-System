#pragma once

#include <QDialog>
#include "ui_AddEditNumberDialog.h"
#include "PhoneNumber.h"

class AddEditNumberDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddEditNumberDialog(QWidget* parent = nullptr);
    explicit AddEditNumberDialog(const PhoneNumber& number, QWidget* parent = nullptr);
    ~AddEditNumberDialog();

    PhoneNumber getPhoneNumber() const;

private slots:
    void validateInput();
    void onAccept();

private:
    Ui::AddEditNumberDialog ui;
    PhoneNumber m_number;
    bool m_editMode = false;

    void setupUI();
    void populateTypeComboBox();
    void populateServiceTypeComboBox();
};
