/*=========================================================================

Copyright (c) 1998-2012 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
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
// .NAME Item.h -
// .SECTION Description
// .SECTION See Also

#ifndef __slctk_model_Item_h
#define __slctk_model_Item_h

#include "ModelExports.h"
#include "PublicPointerDefs.h"
#include <string>

namespace slctk
{
  namespace model
  {
    class Model;
    class SLCTKMODEL_EXPORT Item
    {
      friend class Model;
    public:
      enum Type
      {
        VERTEX = 1,
        EDGE = 2,
        FACE=4,
        REGION=8,
        MODEL_DOMAIN=16,
        GROUP=32,
        AUXILIARY_GEOMETRY=64,
      };
      Item(Model *model, int myid);
      virtual ~Item();
      int id() const
      {return this->m_id;}
      virtual std::string name() const = 0;
      virtual Item::Type type() const = 0;
      Model *model() const
      {return this->m_model;}

    protected:
      void clearModel()
      {this->m_model = NULL;}

      Model *m_model;
      int m_id;
    private:
    };
  };
};


#endif /* __slctk_model_Item_h */
