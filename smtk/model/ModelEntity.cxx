#include "smtk/model/ModelEntity.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/GroupEntity.h"
#include "smtk/model/Storage.h"
#include "smtk/model/Arrangement.h"

namespace smtk {
  namespace model {

Cursor ModelEntity::parent() const
{
  return CursorArrangementOps::firstRelation<Cursor>(*this, EMBEDDED_IN);
}

/// Return the cells directly owned by this model.
CellEntities ModelEntity::cells() const
{
  CellEntities result;
  CursorArrangementOps::appendAllRelations(*this, INCLUDES, result);
  return result;
}

/// Return the groups directly owned by this model.
GroupEntities ModelEntity::groups() const
{
  GroupEntities result;
  CursorArrangementOps::appendAllRelations(*this, INCLUDES, result);
  return result;
}

/// Return the models directly owned by this model.
ModelEntities ModelEntity::models() const
{
  ModelEntities result;
  CursorArrangementOps::appendAllRelations(*this, INCLUDES, result);
  return result;
}

  } // namespace model
} // namespace smtk