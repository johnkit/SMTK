#include "smtk/Qt/qtEntityItemModel.h"

#include "smtk/model/Entity.h"
#include "smtk/model/EntityPhrase.h"
#include "smtk/model/FloatData.h"
#include "smtk/model/IntegerData.h"
#include "smtk/model/Storage.h"
#include "smtk/model/StringData.h"

#include <QtCore/QDir>
#include <QtCore/QDirIterator>
#include <QtCore/QFile>
#include <QtCore/QVariant>

#include <iomanip>
#include <sstream>
#include <deque>

// -----------------------------------------------------------------------------
// The following is used to ensure that the QRC file
// containing the entity-type icons is registered.
// This is required when building SMTK with static libraries.
// Note that the inlined functions may *NOT* be placed inside a namespace
// according to the Qt docs here:
//   http://qt-project.org/doc/qt-5.0/qtcore/qdir.html#Q_INIT_RESOURCE
static int resourceCounter = 0;

inline void initIconResource()
{
  if (resourceCounter <= 0)
    {
    Q_INIT_RESOURCE(qtEntityItemModelIcons);
    }
  ++resourceCounter;
}

inline void cleanupIconResource()
{
  if (--resourceCounter == 0)
    {
    Q_CLEANUP_RESOURCE(qtEntityItemModelIcons);
    }
}
// -----------------------------------------------------------------------------

namespace smtk {
  namespace model {

// -----------------------------------------------------------------------------
QEntityItemModel::QEntityItemModel(QObject* owner)
  : QAbstractItemModel(owner)
{
  this->m_deleteOnRemoval = true;
  initIconResource();
}

QEntityItemModel::~QEntityItemModel()
{
  cleanupIconResource();
}

QModelIndex QEntityItemModel::index(int row, int column, const QModelIndex& owner) const
{
  if (owner.isValid() && owner.column() != 0)
    return QModelIndex();

  DescriptivePhrase* ownerPhrase = this->getItem(owner);
  DescriptivePhrases& subphrases(ownerPhrase->subphrases());
  if (row >= 0 && row < static_cast<int>(subphrases.size()))
    {
    //std::cout << "index(_"  << ownerPhrase->title() << "_, " << row << ") = " << subphrases[row]->title() << "\n";
    return this->createIndex(row, column, subphrases[row].get());
    }
  std::cerr << "index(): Bad row " << row << " for owner " << ownerPhrase->title() << "\n";
  return QModelIndex();
}

QModelIndex QEntityItemModel::parent(const QModelIndex& child) const
{
  if (!child.isValid())
    {
    return QModelIndex();
    }

  DescriptivePhrase* childPhrase = this->getItem(child);
  DescriptivePhrasePtr parentPhrase = childPhrase->parent();
  if (parentPhrase == this->m_root)
    {
    return QModelIndex();
    }

  int childRow = parentPhrase ? parentPhrase->argFindChild(childPhrase) : -1;
  if (childRow < 0)
    {
    std::cerr << "parent(): could not find child " << childPhrase->title() << " in parent " << parentPhrase->title() << "\n";
    return QModelIndex();
    }
  return this->createIndex(childRow, 0, parentPhrase.get());
}

/// Return true when \a owner has subphrases.
bool QEntityItemModel::hasChildren(const QModelIndex& owner) const
{
  // According to various Qt mailing-list posts, we might
  // speed things up by always returning true here.
  if (owner.isValid())
    {
    DescriptivePhrase* phrase =
      static_cast<DescriptivePhrase*>(owner.internalPointer());
    if (phrase)
      { // Return whether the parent has subphrases.
      return phrase->subphrases().empty() ? false : true;
      }
    }
  // Return whether the toplevel m_phrases list is empty.
  return (this->m_root ?
    (this->m_root->subphrases().empty() ? false : true) :
    false);
}

/// The number of rows in the table "underneath" \a owner.
int QEntityItemModel::rowCount(const QModelIndex& owner) const
{
  DescriptivePhrase* ownerPhrase = this->getItem(owner);
  return static_cast<int>(ownerPhrase->subphrases().size());
}

/// Return something to display in the table header.
QVariant QEntityItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole)
    {
    return QVariant();
    }
  if (orientation == Qt::Horizontal)
    {
    switch (section)
      {
    case 0:
      return "Entity"; // "Type";
    case 1:
      return "Dimension";
    case 2:
      return "Name";
      }
    }
  // ... could add "else" here to present per-row header.
  return QVariant();
}

/// Relate information, by its \a role, from a \a DescriptivePhrase to the Qt model.
QVariant QEntityItemModel::data(const QModelIndex& idx, int role) const
{
  if (idx.isValid())
    {
    // A valid index may have a *parent* that is:
    // + invalid (in which case we use row() as an offset into m_phrases to obtain data)
    // + valid (in which case it points to a DescriptivePhrase instance and row() is an offset into the subphrases)

    DescriptivePhrase* item =
      static_cast<DescriptivePhrase*>(idx.internalPointer());
    if (item)
      {
      if (role == TitleTextRole)
        {
        return QVariant(item->title().c_str());
        }
      else if (role == SubtitleTextRole)
        {
        return QVariant(item->subtitle().c_str());
        }
      else if (role == EntityIconRole)
        {
        return QVariant(
          this->lookupIconForEntityFlags(
            item->phraseType()));
        }
      else if (role == EntityColorRole)
        {
        QColor color;
        FloatList rgba = item->relatedColor();
        if (rgba.size() >= 4 && rgba[3] < 0)
          color = QColor(255, 255, 255, 255);
        else
          {
          // Color may be luminance, luminance+alpha, rgb, or rgba:
          switch (rgba.size())
            {
          case 0:
            color = QColor(0, 0, 0, 0);
            break;
          case 1:
            color.setHslF(0., 0., rgba[0], 1.);
            break;
          case 2:
            color.setHslF(0., 0., rgba[0], rgba[1]);
            break;
          case 3:
            color.setRgbF(rgba[0], rgba[1], rgba[2], 1.);
            break;
          case 4:
          default:
            color.setRgbF(rgba[0], rgba[1], rgba[2], rgba[3]);
            break;
            }
          }
        return color;
        }
      }
    }
  return QVariant();
}

/*
bool QEntityItemModel::insertRows(int position, int rows, const QModelIndex& owner)
{
  (void)owner;
  beginInsertRows(QModelIndex(), position, position + rows - 1);

  int maxPos = position + rows;
  for (int row = position; row < maxPos; ++row)
    {
    smtk::util::UUID uid = this->m_storage->addEntityOfTypeAndDimension(smtk::model::INVALID, -1);
    this->m_phrases.insert(this->m_phrases.begin() + row, uid);
    this->m_reverse[uid] = row;
    }

  endInsertRows();
  return true;
}

bool QEntityItemModel::removeRows(int position, int rows, const QModelIndex& owner)
{
  (void)owner;
  beginRemoveRows(QModelIndex(), position, position + rows - 1);

  smtk::util::UUIDArray uids(this->m_phrases.begin() + position, this->m_phrases.begin() + position + rows);
  this->m_phrases.erase(this->m_phrases.begin() + position, this->m_phrases.begin() + position + rows);
  for (smtk::util::UUIDArray::const_iterator it = uids.begin(); it != uids.end(); ++it)
    {
    this->m_reverse.erase(this->m_reverse.find(*it));
    if (this->m_deleteOnRemoval)
      {
      this->m_storage->erase(*it);
      // FIXME: Should we keep a set of all it->second.relations()
      //        and emit "didChange" events for all the relations
      //        that were affected by removal of this entity?
      }
    }

  endRemoveRows();
  return true;
}
*/

bool QEntityItemModel::setData(const QModelIndex& idx, const QVariant& value, int role)
{
  bool didChange = false;
  DescriptivePhrase* phrase;
  if(idx.isValid() &&
    (phrase = static_cast<DescriptivePhrase*>(idx.internalPointer())))
    {
    if (role == TitleTextRole && phrase->isTitleMutable())
      {
      std::string sval = value.value<QString>().toStdString();
      didChange = phrase->setTitle(sval);
      }
    else if (role == SubtitleTextRole && phrase->isSubtitleMutable())
      {
      std::string sval = value.value<QString>().toStdString();
      didChange = phrase->setSubtitle(sval);
      }
    else if (role == EntityColorRole && phrase->isRelatedColorMutable())
      {
      QColor color = value.value<QColor>();
      FloatList rgba(4);
      rgba[0] = color.redF();
      rgba[1] = color.greenF();
      rgba[2] = color.blueF();
      rgba[3] = color.alphaF();
      didChange = phrase->setRelatedColor(rgba);
      }
    }
  return didChange;
}

/*
void QEntityItemModel::sort(int column, Qt::SortOrder order)
{
  switch (column)
    {
  case -1:
      { // Sort by UUID.
      std::multiset<smtk::util::UUID> sorter;
      this->sortDataWithContainer(sorter, order);
      }
    break;
  case 1:
      {
      smtk::model::SortByEntityFlags comparator(this->m_storage);
      std::multiset<smtk::util::UUID,smtk::model::SortByEntityFlags>
        sorter(comparator);
      this->sortDataWithContainer(sorter, order);
      }
    break;
  case 0:
  case 2:
      {
      smtk::model::SortByEntityProperty<
        smtk::model::BRepModel,
        smtk::model::StringList,
        &smtk::model::BRepModel::stringProperty,
        &smtk::model::BRepModel::hasStringProperty> comparator(
          this->m_storage, "name");
      std::multiset<
        smtk::util::UUID,
        smtk::model::SortByEntityProperty<
          smtk::model::BRepModel,
          smtk::model::StringList,
          &smtk::model::BRepModel::stringProperty,
          &smtk::model::BRepModel::hasStringProperty> >
        sorter(comparator);
      this->sortDataWithContainer(sorter, order);
      }
    break;
  default:
    std::cerr << "Bad column " << column << " for sorting\n";
    break;
    }
  emit dataChanged(
    this->index(0, 0, QModelIndex()),
    this->index(
      static_cast<int>(this->m_phrases.size()),
      this->columnCount(),
      QModelIndex()));
}
*/

/// Provide meta-information about a model entry.
Qt::ItemFlags QEntityItemModel::flags(const QModelIndex& idx) const
{
  if(!idx.isValid())
    return Qt::ItemIsEnabled;

  // TODO: Check to make sure column is not "information-only".
  //       We don't want to allow people to randomly edit an enum string.
  Qt::ItemFlags itemFlags = QAbstractItemModel::flags(idx) |
     Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
  DescriptivePhrase* dp = this->getItem(idx);
  if( dp && dp->relatedEntity().isGroupEntity() )
    itemFlags = itemFlags | Qt::ItemIsDropEnabled;
  return itemFlags;
}

/**\brief Return the first smtk::model::Storage instance presented by this model.
  *
  * Note that it is possible for a QEntityItemModel to present information
  * on entities from multiple Storage instances.
  * However, in this case, external updates to the selection must either be
  * made via Cursor instances (which couple UUIDs with Storage instances) or
  * there will be breakage.
  */
smtk::model::StoragePtr QEntityItemModel::storage() const
{
  if (this->m_root)
    {
    Cursor related = this->m_root->relatedEntity();
    if (related.isValid())
      return related.storage();
    // Keep looking until we find a phrase with a valid
    // relatedEntity (which must have valid storage).
    std::deque<DescriptivePhrase::Ptr> phrases(
      this->m_root->subphrases().begin(),
      this->m_root->subphrases().end());
    while (!phrases.empty())
      {
      related = phrases.front()->relatedEntity();
      if (related.isValid())
        return related.storage();
      DescriptivePhrases ptmp = phrases.front()->subphrases();
      phrases.pop_front();
      phrases.insert(phrases.end(), ptmp.begin(), ptmp.end());
      }
    }

  smtk::model::StoragePtr null;
  return null;
}

QIcon QEntityItemModel::lookupIconForEntityFlags(unsigned long flags)
{
  std::ostringstream resourceName;
  resourceName << ":/icons/entityTypes/";
  bool dimBits = true;
  switch (flags & ENTITY_MASK)
    {
  case CELL_ENTITY:
    resourceName << "cell";
    break;
  case USE_ENTITY:
    resourceName << "use";
    break;
  case SHELL_ENTITY:
    resourceName << "shell";
    break;
  case GROUP_ENTITY:
    resourceName << "group";
    break;
  case MODEL_ENTITY:
    resourceName << "model";
    break;
  case INSTANCE_ENTITY:
    resourceName << "instance";
    break;
  default:
    resourceName << "invalid";
    dimBits = false;
    break;
    }
  if (dimBits)
    {
    resourceName
      << "_"
      << std::setbase(16) << std::fixed << std::setw(2) << std::setfill('0')
      << (flags & ANY_DIMENSION)
      ;
    }
  resourceName << ".svg";
  QFile rsrc(resourceName.str().c_str());
  if (!rsrc.exists())
    { // FIXME: Replace with the path of a "generic entity" or "invalid" icon.
    return QIcon(":/icons/entityTypes/cell_08.svg");
    }
  return QIcon(resourceName.str().c_str());
}

/**\brief Sort the UUIDs being displayed using the given ordered container.
  *
  * The ordered container's comparator is used to insertion-sort the UUIDs
  * displayed. Then, the \a order is used to either forward- or reverse-iterator
  * over the container to obtain a new ordering for the UUIDs.
template<typename T>
void QEntityItemModel::sortDataWithContainer(T& sorter, Qt::SortOrder order)
{
  smtk::util::UUIDArray::iterator ai;
  // Insertion into the set sorts the UUIDs.
  for (ai = this->m_phrases.begin(); ai != this->m_phrases.end(); ++ai)
    {
    sorter.insert(*ai);
    }
  // Now we reset m_phrases and m_reverse and recreate based on the sorter's order.
  this->m_phrases.clear();
  this->m_reverse.clear();
  int i;
  if (order == Qt::AscendingOrder)
    {
    typename T::iterator si;
    for (i = 0, si = sorter.begin(); si != sorter.end(); ++si, ++i)
      {
      this->m_phrases.push_back(*si);
      this->m_reverse[*si] = i;
      / *
      std::cout << i << "  " << *si << "  " <<
        (this->m_storage->hasStringProperty(*si, "name") ?
         this->m_storage->stringProperty(*si, "name")[0].c_str() : "--") << "\n";
         * /
      }
    }
  else
    {
    typename T::reverse_iterator si;
    for (i = 0, si = sorter.rbegin(); si != sorter.rend(); ++si, ++i)
      {
      this->m_phrases.push_back(*si);
      this->m_reverse[*si] = i;
      / *
      std::cout << i << "  " << *si << "  " <<
        (this->m_storage->hasStringProperty(*si, "name") ?
         this->m_storage->stringProperty(*si, "name")[0].c_str() : "--") << "\n";
         * /
      }
    }
}
  */

/// A utility function to retrieve the DescriptivePhrase associated with a model index.
DescriptivePhrase* QEntityItemModel::getItem(const QModelIndex& idx) const
{
  if (idx.isValid())
    {
    DescriptivePhrase* phrase = static_cast<DescriptivePhrase*>(idx.internalPointer());
    if (phrase)
      {
      return phrase;
      }
    }
  return this->m_root.get();
}

  } // namespace model
} // namespace smtk
