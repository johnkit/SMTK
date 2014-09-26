//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_vtk_ModelMultiBlockSource_h
#define __smtk_vtk_ModelMultiBlockSource_h

#include "smtk/extension/vtk/vtkSMTKExports.h"
#include "vtkMultiBlockDataSetAlgorithm.h"
#include "smtk/PublicPointerDefs.h"

#include <map>

class vtkPolyData;

/**\brief A VTK source for exposing model geometry in SMTK Manager as multiblock data.
  *
  * This filter generates a single block per UUID, for every UUID
  * in model manager with a tessellation entry.
  */
class VTKSMTK_EXPORT vtkModelMultiBlockSource : public vtkMultiBlockDataSetAlgorithm
{
public:
  static vtkModelMultiBlockSource* New();
  virtual void PrintSelf(ostream& os, vtkIndent indent);
  vtkTypeMacro(vtkModelMultiBlockSource,vtkMultiBlockDataSetAlgorithm);

  vtkGetObjectMacro(CachedOutput,vtkMultiBlockDataSet);

  smtk::model::ManagerPtr GetModelManager();
  void SetModelManager(smtk::model::ManagerPtr);

  void GetUUID2BlockIdMap(std::map<std::string, unsigned int>& uuid2mid);
  void Dirty();

  vtkGetVector4Macro(DefaultColor,double);
  vtkSetVector4Macro(DefaultColor,double);

protected:
  vtkModelMultiBlockSource();
  virtual ~vtkModelMultiBlockSource();

  void GenerateRepresentationFromModelEntity(
    vtkPolyData* poly, const smtk::model::Cursor& entity);
  void GenerateRepresentationFromModel(
    vtkMultiBlockDataSet* mbds, smtk::model::ManagerPtr model);

  //virtual int FillInputPortInformation(int port, vtkInformation* request);
  //virtual int FillOutputPortInformation(int port, vtkInformation* request);

  virtual int RequestData(
    vtkInformation* request,
    vtkInformationVector** inInfo,
    vtkInformationVector* outInfo);

  void SetCachedOutput(vtkMultiBlockDataSet*);

  // Instance model Manager:
  smtk::model::ManagerPtr ModelMgr;
  vtkMultiBlockDataSet* CachedOutput;
  double DefaultColor[4];

  // Internal map of UUID and block index into multiblock
  std::map<std::string, unsigned int> UUID2BlockIdMap;

private:

  vtkModelMultiBlockSource(const vtkModelMultiBlockSource&); // Not implemented.
  void operator = (const vtkModelMultiBlockSource&); // Not implemented.
};

#endif // __smtk_vtk_ModelMultiBlockSource_h