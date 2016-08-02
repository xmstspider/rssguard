// This file is part of RSS Guard.
//
// Copyright (C) 2011-2016 by Martin Rotter <rotter.martinos@gmail.com>
//
// RSS Guard is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// RSS Guard is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with RSS Guard. If not, see <http://www.gnu.org/licenses/>.

#include "gui/settings/settingsdatabase.h"

#include "miscellaneous/databasefactory.h"
#include "definitions/definitions.h"
#include "miscellaneous/application.h"
#include "miscellaneous/textfactory.h"


SettingsDatabase::SettingsDatabase(Settings *settings, QWidget *parent)
  : SettingsPanel(settings, parent), m_ui(new Ui::SettingsDatabase) {
  m_ui->setupUi(this);

  connect(m_ui->m_cmbDatabaseDriver, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &SettingsDatabase::selectSqlBackend);
  connect(m_ui->m_checkMysqlShowPassword, &QCheckBox::toggled, this, &SettingsDatabase::switchMysqlPasswordVisiblity);
  connect(m_ui->m_txtMysqlUsername->lineEdit(), &BaseLineEdit::textChanged, this, &SettingsDatabase::onMysqlUsernameChanged);
  connect(m_ui->m_txtMysqlHostname->lineEdit(), &BaseLineEdit::textChanged, this, &SettingsDatabase::onMysqlHostnameChanged);
  connect(m_ui->m_txtMysqlPassword->lineEdit(), &BaseLineEdit::textChanged, this, &SettingsDatabase::onMysqlPasswordChanged);
  connect(m_ui->m_txtMysqlDatabase->lineEdit(), &BaseLineEdit::textChanged, this, &SettingsDatabase::onMysqlDatabaseChanged);
  connect(m_ui->m_btnMysqlTestSetup, &QPushButton::clicked, this, &SettingsDatabase::mysqlTestConnection);
  connect(m_ui->m_spinMysqlPort, &QSpinBox::editingFinished, this, &SettingsDatabase::requireRestart);
  connect(m_ui->m_txtMysqlHostname->lineEdit(), &BaseLineEdit::textEdited, this, &SettingsDatabase::requireRestart);
  connect(m_ui->m_txtMysqlPassword->lineEdit(), &BaseLineEdit::textEdited, this, &SettingsDatabase::requireRestart);
  connect(m_ui->m_txtMysqlUsername->lineEdit(), &BaseLineEdit::textEdited, this, &SettingsDatabase::requireRestart);
}

SettingsDatabase::~SettingsDatabase() {
  delete m_ui;
}

void SettingsDatabase::mysqlTestConnection() {
  const DatabaseFactory::MySQLError error_code = qApp->database()->mysqlTestConnection(m_ui->m_txtMysqlHostname->lineEdit()->text(),
                                                                                       m_ui->m_spinMysqlPort->value(),
                                                                                       m_ui->m_txtMysqlDatabase->lineEdit()->text(),
                                                                                       m_ui->m_txtMysqlUsername->lineEdit()->text(),
                                                                                       m_ui->m_txtMysqlPassword->lineEdit()->text());
  const QString interpretation = qApp->database()->mysqlInterpretErrorCode(error_code);


  switch (error_code) {
    case DatabaseFactory::MySQLOk:
    case DatabaseFactory::MySQLUnknownDatabase:
      m_ui->m_lblMysqlTestResult->setStatus(WidgetWithStatus::Ok, interpretation, interpretation);
      break;

    default:
      m_ui->m_lblMysqlTestResult->setStatus(WidgetWithStatus::Error, interpretation, interpretation);
      break;
  }
}

void SettingsDatabase::onMysqlHostnameChanged(const QString &new_hostname) {
  if (new_hostname.isEmpty()) {
    m_ui->m_txtMysqlHostname->setStatus(LineEditWithStatus::Warning, tr("Hostname is empty."));
  }
  else {
    m_ui->m_txtMysqlHostname->setStatus(LineEditWithStatus::Ok, tr("Hostname looks ok."));
  }
}

void SettingsDatabase::onMysqlUsernameChanged(const QString &new_username) {
  if (new_username.isEmpty()) {
    m_ui->m_txtMysqlUsername->setStatus(LineEditWithStatus::Warning, tr("Username is empty."));
  }
  else {
    m_ui->m_txtMysqlUsername->setStatus(LineEditWithStatus::Ok, tr("Username looks ok."));
  }
}

void SettingsDatabase::onMysqlPasswordChanged(const QString &new_password) {
  if (new_password.isEmpty()) {
    m_ui->m_txtMysqlPassword->setStatus(LineEditWithStatus::Warning, tr("Password is empty."));
  }
  else {
    m_ui->m_txtMysqlPassword->setStatus(LineEditWithStatus::Ok, tr("Password looks ok."));
  }
}

void SettingsDatabase::onMysqlDatabaseChanged(const QString &new_database) {
  if (new_database.isEmpty()) {
    m_ui->m_txtMysqlDatabase->setStatus(LineEditWithStatus::Warning, tr("Working database is empty."));
  }
  else {
    m_ui->m_txtMysqlDatabase->setStatus(LineEditWithStatus::Ok, tr("Working database is ok."));
  }
}

void SettingsDatabase::selectSqlBackend(int index) {
  const QString selected_db_driver = m_ui->m_cmbDatabaseDriver->itemData(index).toString();

  if (selected_db_driver == APP_DB_SQLITE_DRIVER) {
    m_ui->m_stackedDatabaseDriver->setCurrentIndex(0);
  }
  else if (selected_db_driver == APP_DB_MYSQL_DRIVER) {
    m_ui->m_stackedDatabaseDriver->setCurrentIndex(1);
  }
  else {
    qWarning("GUI for given database driver '%s' is not available.", qPrintable(selected_db_driver));
  }
}

void SettingsDatabase::switchMysqlPasswordVisiblity(bool visible) {
  m_ui->m_txtMysqlPassword->lineEdit()->setEchoMode(visible ? QLineEdit::Normal : QLineEdit::Password);
}

void SettingsDatabase::loadSettings() {
  onBeginLoadSettings();

  m_ui->m_lblMysqlTestResult->setStatus(WidgetWithStatus::Information,  tr("No connection test triggered so far."), tr("You did not executed any connection test yet."));

  // Load SQLite.
  m_ui->m_cmbDatabaseDriver->addItem(qApp->database()->humanDriverName(DatabaseFactory::SQLITE), APP_DB_SQLITE_DRIVER);

  // Load in-memory database status.
  m_ui->m_checkSqliteUseInMemoryDatabase->setChecked(settings()->value(GROUP(Database), SETTING(Database::UseInMemory)).toBool());

  if (QSqlDatabase::isDriverAvailable(APP_DB_MYSQL_DRIVER)) {
    onMysqlHostnameChanged(QString());
    onMysqlUsernameChanged(QString());
    onMysqlPasswordChanged(QString());
    onMysqlDatabaseChanged(QString());

    // Load MySQL.
    m_ui->m_cmbDatabaseDriver->addItem(qApp->database()->humanDriverName(DatabaseFactory::MYSQL), APP_DB_MYSQL_DRIVER);

    // Setup placeholders.
    m_ui->m_txtMysqlHostname->lineEdit()->setPlaceholderText(tr("Hostname of your MySQL server"));
    m_ui->m_txtMysqlUsername->lineEdit()->setPlaceholderText(tr("Username to login with"));
    m_ui->m_txtMysqlPassword->lineEdit()->setPlaceholderText(tr("Password for your username"));
    m_ui->m_txtMysqlDatabase->lineEdit()->setPlaceholderText(tr("Working database which you have full access to."));

    m_ui->m_txtMysqlHostname->lineEdit()->setText(settings()->value(GROUP(Database), SETTING(Database::MySQLHostname)).toString());
    m_ui->m_txtMysqlUsername->lineEdit()->setText(settings()->value(GROUP(Database), SETTING(Database::MySQLUsername)).toString());
    m_ui->m_txtMysqlPassword->lineEdit()->setText(TextFactory::decrypt(settings()->value(GROUP(Database), SETTING(Database::MySQLPassword)).toString()));
    m_ui->m_txtMysqlDatabase->lineEdit()->setText(settings()->value(GROUP(Database), SETTING(Database::MySQLDatabase)).toString());
    m_ui->m_spinMysqlPort->setValue(settings()->value(GROUP(Database), SETTING(Database::MySQLPort)).toInt());

    m_ui->m_checkMysqlShowPassword->setChecked(false);
  }

  int index_current_backend = m_ui->m_cmbDatabaseDriver->findData(settings()->value(GROUP(Database), SETTING(Database::ActiveDriver)).toString());

  if (index_current_backend >= 0) {
    m_ui->m_cmbDatabaseDriver->setCurrentIndex(index_current_backend);
  }

  onEndLoadSettings();
}

void SettingsDatabase::saveSettings() {
  onBeginSaveSettings();

  // Setup in-memory database status.
  const bool original_inmemory = settings()->value(GROUP(Database), SETTING(Database::UseInMemory)).toBool();
  const bool new_inmemory = m_ui->m_checkSqliteUseInMemoryDatabase->isChecked();

  // Save data storage settings.
  QString original_db_driver = settings()->value(GROUP(Database), SETTING(Database::ActiveDriver)).toString();
  QString selected_db_driver = m_ui->m_cmbDatabaseDriver->itemData(m_ui->m_cmbDatabaseDriver->currentIndex()).toString();

  // Save SQLite.
  settings()->setValue(GROUP(Database), Database::UseInMemory, new_inmemory);

  if (QSqlDatabase::isDriverAvailable(APP_DB_MYSQL_DRIVER)) {
    // Save MySQL.
    settings()->setValue(GROUP(Database), Database::MySQLHostname, m_ui->m_txtMysqlHostname->lineEdit()->text());
    settings()->setValue(GROUP(Database), Database::MySQLUsername, m_ui->m_txtMysqlUsername->lineEdit()->text());
    settings()->setValue(GROUP(Database), Database::MySQLPassword, TextFactory::encrypt(m_ui->m_txtMysqlPassword->lineEdit()->text()));
    settings()->setValue(GROUP(Database), Database::MySQLDatabase, m_ui->m_txtMysqlDatabase->lineEdit()->text());
    settings()->setValue(GROUP(Database), Database::MySQLPort, m_ui->m_spinMysqlPort->value());
  }

  settings()->setValue(GROUP(Database), Database::ActiveDriver, selected_db_driver);

  if (original_db_driver != selected_db_driver || original_inmemory != new_inmemory) {
    requireRestart();
  }

  onEndSaveSettings();
}
