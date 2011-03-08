/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/
#include "vtkCmbModelEdgeMesh.h"

#include "vtkCmbMesh.h"
#include "vtkCmbModelVertexMesh.h"
#include <vtkModel.h>
#include <vtkModelEdge.h>
#include <vtkModelItemIterator.h>
#include <vtkModelVertex.h>
#include <vtkPolyData.h>

vtkCxxRevisionMacro(vtkCmbModelEdgeMesh, "");

//----------------------------------------------------------------------------
vtkCmbModelEdgeMesh::vtkCmbModelEdgeMesh()
{
  this->ModelEdge = NULL;
  this->Length = 0;
  this->MeshedLength = 0;
}

//----------------------------------------------------------------------------
vtkCmbModelEdgeMesh::~vtkCmbModelEdgeMesh()
{
}

//----------------------------------------------------------------------------
vtkModelGeometricEntity* vtkCmbModelEdgeMesh::GetModelGeometricEntity()
{
  return this->ModelEdge;
}

//----------------------------------------------------------------------------
void vtkCmbModelEdgeMesh::Initialize(vtkCmbMesh* masterMesh, vtkModelEdge* edge)
{
  if(masterMesh == NULL || edge == NULL)
    {
    vtkErrorMacro("Passed in masterMesh or edge is NULL.");
    return;
    }
  this->SetMasterMesh(masterMesh);
  this->ModelEdge = edge;
  this->SetLength(0);
  this->SetMeshedLength(0);
  this->SetModelEntityMesh(NULL);
}

//----------------------------------------------------------------------------
bool vtkCmbModelEdgeMesh::BuildModelEntityMesh(
  bool meshHigherDimensionalEntities)
{
  if(!this->ModelEdge)
    {
    return false;
    }
  bool doBuild = false;
  if(this->GetModelEntityMesh() == NULL && this->GetActualLength() > 0.)
    {
    doBuild = true;
    }
  else if(this->MeshedLength != this->GetActualLength())
    {
    doBuild = true;
    }
  else if( vtkPolyData* modelGeometry =
           vtkPolyData::SafeDownCast(this->ModelEdge->GetGeometry()) )
    {
    if(modelGeometry->GetMTime() >= this->GetModelEntityMesh()->GetMTime())
      { // if the model poly is newer than the mesh poly we need to remesh
      doBuild = true;
      }
    }
  if(doBuild == false)
    {
    return true;
    }
  return this->BuildMesh(meshHigherDimensionalEntities);
}

//----------------------------------------------------------------------------
vtkCmbModelVertexMesh* vtkCmbModelEdgeMesh::GetAdjacentModelVertexMesh(
  int which)
{
  if(this->ModelEdge == NULL || this->GetMasterMesh() == NULL)
    {
    vtkErrorMacro("Must initialize before using object.");
    return NULL;
    }
  vtkModelVertex* modelVertex = this->ModelEdge->GetAdjacentModelVertex(which);
  return vtkCmbModelVertexMesh::SafeDownCast(
    this->GetMasterMesh()->GetModelEntityMesh(modelVertex));
}

//----------------------------------------------------------------------------
double vtkCmbModelEdgeMesh::GetActualLength()
{
  double actualLength = this->Length;
  double globalLength = this->GetMasterMesh()->GetGlobalLength();
  if(globalLength > 0. && globalLength < actualLength)
    {
    actualLength = globalLength;
    }
  else if(actualLength == 0)
    {
    actualLength = globalLength;
    }
  return actualLength;
}

//----------------------------------------------------------------------------
void vtkCmbModelEdgeMesh::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  if(this->ModelEdge)
    {
    os << indent << "ModelEdge: " << this->ModelEdge << "\n";
    }
  else
    {
    os << indent << "ModelEdge: (NULL)\n";
    }
  os << indent << "Length: " << this->Length << "\n";
  os << indent << "MeshedLength: " << this->MeshedLength << "\n";
}

