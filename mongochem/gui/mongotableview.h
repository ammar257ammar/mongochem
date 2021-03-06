/******************************************************************************

  This source file is part of the MongoChem project.

  Copyright 2011 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#ifndef MongoTableView_H
#define MongoTableView_H

#include <QtWidgets/QTableView>

class QNetworkAccessManager;
class QNetworkReply;

namespace MongoChem {

class MoleculeRef;
class OpenInEditorHandler;

class MongoTableView : public QTableView
{
  Q_OBJECT

public:
  MongoTableView(QWidget *parent = 0);

  /** Custom context menu for this view. */
  void contextMenuEvent(QContextMenuEvent *e);

signals:
  /** Emitted when the user queries for molecules similar to @p ref. */
  void showSimilarMolecules(MongoChem::MoleculeRef ref);

  /** Emitted when the user requests the molecule details for @p ref either
   *  by clicking "Show Detail" or double-clicking on the molecule. */
  void showMoleculeDetails(MongoChem::MoleculeRef ref);

protected slots:
  /** Attempt to fetch the structure of a CAS number from the NIH resolver. */
  void fetchByCas();

  /** Attempt to fetch the IUPAC name of a structure from the NIH resolver. */
  void fetchIUPAC();

  /** Attempt to fetch a PNG of the molecule. */
  void fetchImage();

  /** Show the molecule details dialog. */
  void showMoleculeDetailsDialog();

  /** Copies the InChI formula to the clipboard. */
  void copyInChIToClipboard();

  /** Retrieving structures from remote databases! */
  void replyFinished(QNetworkReply*);

  /** Query the database for similar molecules. */
  void showSimilarMoleculesClicked();

  /** Called when the user double clicks on a molecule. */
  void moleculeDoubleClicked(const QModelIndex &index);

  /**
   * Called when the vertical scroll bar changes.
   */
  void scollBarMoved(int value);

  /**
   * Called when the user right-clicks on a column header.
   */
  void columnHeaderCustomContextMenuRequested(const QPoint &pos);

  /**
   * Called when the user toggles the visibility of a column header.
   */
  void headerItemVisibilityToggled();

  /**
   * Called when the user clicks on a column header.
   */
  void headerItemClicked(int index);

private:
  /** Returns the currently selected model index. */
  QModelIndex currentSourceModelIndex() const;

protected:
  /** Used for fetching data from web services. */
  QNetworkAccessManager *m_network;
  QString m_moleculeName;
  int m_row;
  OpenInEditorHandler *m_openInEditorHandler;
};

} // end MongoChem namespace

#endif
