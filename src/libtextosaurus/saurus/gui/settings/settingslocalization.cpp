// This file is distributed under GNU GPLv3 license. For full license text, see <project-git-repository-root-folder>/LICENSE.md.

#include "saurus/gui/settings/settingslocalization.h"

#include "common/miscellaneous/iconfactory.h"
#include "common/miscellaneous/localization.h"
#include "common/miscellaneous/settings.h"
#include "saurus/miscellaneous/application.h"

SettingsLocalization::SettingsLocalization(Settings* settings, QWidget* parent)
  : SettingsPanel(settings, parent) {
  m_ui.setupUi(this);
  m_ui.m_treeLanguages->setColumnCount(2);
  m_ui.m_treeLanguages->setHeaderHidden(false);
  m_ui.m_treeLanguages->setHeaderLabels(QStringList()
                                        << /*: Language column of language list. */ tr("Language")
                                        << /*: Lang. code column of language list. */ tr("Code"));

  // Setup languages.
  m_ui.m_treeLanguages->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
  m_ui.m_treeLanguages->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
  connect(m_ui.m_treeLanguages, &QTreeWidget::currentItemChanged, this, &SettingsLocalization::requireRestart);
  connect(m_ui.m_treeLanguages, &QTreeWidget::currentItemChanged, this, &SettingsLocalization::dirtifySettings);
}

void SettingsLocalization::loadSettings() {
  onBeginLoadSettings();

  foreach (const Language& language, qApp->localization()->installedLanguages()) {
    auto* item = new QTreeWidgetItem(m_ui.m_treeLanguages);

    item->setText(0, language.m_name);
    item->setText(1, language.m_code);
    item->setIcon(0, qApp->icons()->miscIcon(QString(FLAG_ICON_SUBFOLDER) + QDir::separator() + language.m_code));
  }

  m_ui.m_treeLanguages->sortByColumn(0, Qt::AscendingOrder);
  QList<QTreeWidgetItem*> matching_items = m_ui.m_treeLanguages->findItems(qApp->localization()->loadedLanguage(), Qt::MatchContains, 1);

  if (!matching_items.isEmpty()) {
    m_ui.m_treeLanguages->setCurrentItem(matching_items[0]);
  }

  onEndLoadSettings();
}

void SettingsLocalization::saveSettings() {
  onBeginSaveSettings();

  if (m_ui.m_treeLanguages->currentItem() == nullptr) {
    qDebug().noquote() << QSL("No localizations loaded in settings dialog, so no saving for them.");
    return;
  }

  const QString actual_lang = qApp->localization()->loadedLanguage();
  const QString new_lang = m_ui.m_treeLanguages->currentItem()->text(1);

  // Save prompt for restart if language has changed.
  if (new_lang != actual_lang) {
    requireRestart();
    settings()->setValue(GROUP(General), General::Language, new_lang);
  }

  onEndSaveSettings();
}
