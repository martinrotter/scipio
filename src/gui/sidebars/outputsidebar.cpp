// For license of this file, see <project-root-folder>/LICENSE.md.

#include "gui/sidebars/outputsidebar.h"

#include "gui/plaintoolbutton.h"
#include "miscellaneous/application.h"
#include "miscellaneous/iconfactory.h"
#include "network-web/webfactory.h"

#include <QFontDatabase>
#include <QLayout>
#include <QRegularExpression>
#include <QScrollBar>
#include <QTabBar>
#include <QTextBrowser>

OutputSidebar::OutputSidebar(QWidget* parent) : DockWidget(parent), m_currentLevel(QMessageBox::Icon::Information),
  m_txtOutput(nullptr) {
  setWindowTitle(tr("Output"));
}

void OutputSidebar::displayOutput(OutputSource source, const QString& message, QMessageBox::Icon level,
                                  const QUrl& url, std::function<void()> handler) {
  Q_UNUSED(source)

  show();
  raise();

  if (level != m_currentLevel) {
    QColor target_color = colorForLevel(level);
    QTextCharFormat c = m_txtOutput->currentCharFormat();

    c.setForeground(target_color);
    m_txtOutput->setCurrentCharFormat(c);
    m_currentLevel = level;
  }

  QString text_to_insert;

  if (url.isValid()) {
    if (handler) {
      m_handlers.insert(url, handler);
    }

    text_to_insert = QString("<a href=\"%1\">%2</a>").arg(url.toString(), message);
  }
  else {
    if (source == OutputSource::ExternalTool) {
      QRegularExpressionMatch http_regex = QRegularExpression(QSL("https?:\\/\\/(www\\.)?[-a-zA-Z0-9@:%._\\+~#=]"
                                                                  "{2,256}\\.[a-z]{2,6}\\b([-a-zA-Z0-9@:%_"
                                                                  "\\+.~#?&\\/\\/=]*)")).match(message);

      if (http_regex.hasMatch()) {
        text_to_insert = QString("<a href=\"%1\">%2</a>").arg(http_regex.captured(), message);
      }
      else {
        text_to_insert = message;
      }
    }
    else {
      text_to_insert = message;
    }
  }

  if (source == OutputSource::ExternalTool) {
    m_txtOutput->append(QString("%1").arg(text_to_insert));
  }
  else {
    m_txtOutput->append(QString("[%2] %1").arg(text_to_insert, QDateTime::currentDateTime().toString(FORMAT_DATETIME_OUTPUT)));
  }
}

QColor OutputSidebar::colorForLevel(QMessageBox::Icon level) {
  switch (level) {
    case QMessageBox::Icon::Information:
      return Qt::GlobalColor::blue;

    case QMessageBox::Icon::Warning:
      return QColor(229, 83, 0);

    case QMessageBox::Icon::Critical:
      return Qt::GlobalColor::red;

    default:
      return Qt::GlobalColor::black;
  }
}

Qt::DockWidgetArea OutputSidebar::initialArea() const {
  return Qt::DockWidgetArea::BottomDockWidgetArea;
}

bool OutputSidebar::initiallyVisible() const {
  return true;
}

int OutputSidebar::initialWidth() const {
  return 150;
}

void OutputSidebar::load() {
  if (m_txtOutput == nullptr) {
    m_txtOutput = new QTextBrowser(this);
    m_txtOutput->setPlaceholderText(tr("This sidebar displays output of external tools and some other critical information..."));
    m_txtOutput->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    m_txtOutput->setWordWrapMode(QTextOption::WrapMode::WrapAnywhere);
    m_txtOutput->setReadOnly(true);
    m_txtOutput->setAutoFormatting(QTextEdit::AutoFormattingFlag::AutoNone);
    m_txtOutput->setObjectName(QSL("m_txtOutput"));
    m_txtOutput->setOpenExternalLinks(false);
    m_txtOutput->setOpenLinks(false);
    m_txtOutput->setFont(QFontDatabase::systemFont(QFontDatabase::SystemFont::FixedFont));

    connect(m_txtOutput, &QTextBrowser::anchorClicked, this, [this](const QUrl& url) {
      if (m_handlers.contains(url)) {
        m_handlers.value(url)();
      }
      else {
        qApp->web()->openUrlInExternalBrowser(url.toString());
      }
    });

    setWidget(m_txtOutput);
  }
}
