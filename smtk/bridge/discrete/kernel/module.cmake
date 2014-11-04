vtk_module(vtkSMTKDiscreteModel
  GROUPS
   DiscreteCore
  DEPENDS
    vtkCommonDataModel
    vtkIOCore
    vtkFiltersCore
    vtkRenderingCore
  PRIVATE_DEPENDS
    vtkFiltersSources
    vtkInteractionStyle
    vtkIOXMLParser
    vtkRenderingFreeTypeOpenGL
    vtkRenderingOpenGL
    vtksys
  EXCLUDE_FROM_WRAP_HIERARCHY
)