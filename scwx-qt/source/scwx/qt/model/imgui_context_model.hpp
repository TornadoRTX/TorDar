#pragma once

#include <memory>
#include <string>

#include <QAbstractListModel>

struct ImFontAtlas;
struct ImGuiContext;

namespace scwx::qt::model
{

class ImGuiContextModelImpl;

struct ImGuiContextInfo
{
   size_t        id_ {};
   std::string   name_ {};
   ImGuiContext* context_ {};

   bool operator==(const ImGuiContextInfo& o) const;
};

class ImGuiContextModel : public QAbstractListModel
{
private:
   Q_OBJECT
   Q_DISABLE_COPY(ImGuiContextModel)

public:
   explicit ImGuiContextModel();
   ~ImGuiContextModel();

   int rowCount(const QModelIndex& parent = QModelIndex()) const override;

   QVariant data(const QModelIndex& index,
                 int                role = Qt::DisplayRole) const override;

   QModelIndex IndexOf(const std::string& contextName) const;

   ImGuiContext* CreateContext(const std::string& name);
   void          DestroyContext(const std::string& name);

   void NewFrame();

   std::vector<ImGuiContextInfo> contexts() const;
   ImFontAtlas*                  font_atlas();

   static ImGuiContextModel& Instance();

signals:
   void ContextsUpdated();

private:
   friend class ImGuiContextModelImpl;
   std::unique_ptr<ImGuiContextModelImpl> p;
};

} // namespace scwx::qt::model
