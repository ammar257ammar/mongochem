/******************************************************************************

  This source file is part of the MongoChem project.

  Copyright 2011-2012 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#ifndef SUBSTRUCTUREFILTERMODEL_H
#define SUBSTRUCTUREFILTERMODEL_H

#include <QSortFilterProxyModel>

#include <chemkit/substructurequery.h>

namespace MongoChem {

class SubstructureFilterModel : public QSortFilterProxyModel
{
  Q_OBJECT

public:
  explicit SubstructureFilterModel(QObject *parent = 0);
  ~SubstructureFilterModel();

  /**
   * Set the identifier for the query molecule. If the identifier starts
   * with "InChI=" it will be interpreted as an InChI, otherwise it will
   * be interpreted as a SMILES.
   */
  void setIdentifier(const QString &identifier);

  /** Returns the identifier for the query molecule. */
  QString identifier() const;

protected:
  virtual bool filterAcceptsColumn(int source_column,
                                   const QModelIndex &source_parent) const;

  /**
   * Returns @c true if the molecule in @p sourceRow matches the query
   * substructure.
   */
  virtual bool filterAcceptsRow(int source_row,
                                const QModelIndex &source_parent) const;

private:
  QString m_identifier;
  chemkit::SubstructureQuery m_query;
};

} // end MongoChem namespace

#endif // SUBSTRUCTUREFILTERMODEL_H
