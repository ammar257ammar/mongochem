/******************************************************************************

  This source file is part of the ChemData project.

  Copyright 2012 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include "mongodatabase.h"

#include <QSettings>

// === MongoDatabase ======================================================= //
/// \class MongoDatabase
/// \brief The MongoDatabase class represents a connection to a
///        Mongo database.
///
/// The MongoDatabase class is implemented as a singleton. The static
/// instance() method is used to retrieve a handle to the database.
///
/// The find*() methods in this class allow users to query for molecules
/// using various identifiers and return MoleculeRef objects representing
/// the molecules found.
///
/// The fetch*() methods take MoleculeRef's and return BSONObj's containing
/// the corresponding molecular data.
///
/// \warning The first invocation of \p instance() forms a persistant
///          connection to the mongo database. This method is not reentrant
///          and should be called only from a single thread.

// --- Construction and Destruction ---------------------------------------- //
/// Creates a new mongo database object. This constructor should not
/// be called by users; rather the instance() method should be used
/// to retrieve a handle to the mongo database.
MongoDatabase::MongoDatabase()
{
  m_db = 0;
}

/// Destroys the mongo database object.
MongoDatabase::~MongoDatabase()
{
  delete m_db;
}

/// Returns an instance of the singleton mongo database.
MongoDatabase* MongoDatabase::instance()
{
  static MongoDatabase singleton;

  if (!singleton.isConnected()) {
    // connect to database
    mongo::DBClientConnection *db = new mongo::DBClientConnection;

    QSettings settings;
    std::string host = settings.value("hostname").toString().toStdString();
    try {
      db->connect(host);
      std::cout << "Connected to: " << host << std::endl;
      singleton.m_db = db;
    }
    catch (mongo::DBException &e) {
      std::cerr << "Failed to connect to MongoDB at '"
                << host
                << "': "
                << e.what()
                << std::endl;
      delete db;
    }
  }

  return &singleton;
}

/// Returns \c true if the database object is connected to the mongo
/// database server.
bool MongoDatabase::isConnected() const
{
  return m_db != 0;
}

// --- Querying ------------------------------------------------------------ //
/// Returns a molecule ref corresponding to the molecule with \p inchi.
MoleculeRef MongoDatabase::findMoleculeFromInChI(const std::string &inchi)
{
  if (!m_db)
    return MoleculeRef();

  std::string collection = moleculesCollectionName();

  mongo::BSONObj obj = m_db->findOne(collection, QUERY("inchi" << inchi));

  return createMoleculeRefForBSONObj(obj);
}

/// Returns a molecule ref corresponding to the molecule with \p inchikey.
MoleculeRef MongoDatabase::findMoleculeFromInChIKey(const std::string &inchikey)
{
  if (!m_db)
    return MoleculeRef();

  std::string collection = moleculesCollectionName();

  mongo::BSONObj obj = m_db->findOne(collection, QUERY("inchikey" << inchikey));

  return createMoleculeRefForBSONObj(obj);
}

/// Returns a molecule ref corresponding to the molecule represented
/// by \p obj.
MoleculeRef MongoDatabase::findMoleculeFromBSONObj(const mongo::BSONObj *obj)
{
  if (!m_db)
    return MoleculeRef();

  mongo::BSONElement inchikeyElement = obj->getField("inchikey");
  if (inchikeyElement.eoo())
    return MoleculeRef();

  return findMoleculeFromInChIKey(inchikeyElement.str());
}

// --- Fetching ------------------------------------------------------------ //
/// Returns a BSONObj containing the data for the molecule referenced
/// by \p molecule.
mongo::BSONObj MongoDatabase::fetchMolecule(const MoleculeRef &molecule)
{
  if (!m_db)
    return mongo::BSONObj();

  std::string collection = moleculesCollectionName();

  return m_db->findOne(collection, QUERY("_id" << molecule.id()));
}

/// Returns a vector of BSONObj's containing the data for the molecules
/// referenced by \p molecules.
std::vector<mongo::BSONObj> MongoDatabase::fetchMolecules(const std::vector<MoleculeRef> &molecules)
{
  std::vector<mongo::BSONObj> objs;

  for(size_t i = 0; i < molecules.size(); i++){
    objs.push_back(fetchMolecule(molecules[i]));
  }

  return objs;
}

/// Creates a new molecule object for \p ref. The ownership of the returned
/// molecule object is passed to the caller.
boost::shared_ptr<chemkit::Molecule> MongoDatabase::createMolecule(const MoleculeRef &ref)
{
  if (!ref.isValid())
    return boost::shared_ptr<chemkit::Molecule>();

  // fetch molecule object
  mongo::BSONObj obj = fetchMolecule(ref);

  // get inchi formula
  mongo::BSONElement inchiElement = obj.getField("inchi");
  if (inchiElement.eoo())
    return boost::shared_ptr<chemkit::Molecule>();

  std::string inchi = inchiElement.str();

  // create molecule from inchi
  chemkit::Molecule *molecule = new chemkit::Molecule(inchi, "inchi");

  // set molecule name
  mongo::BSONElement nameElement = obj.getField("name");
  if (!nameElement.eoo())
    molecule->setName(nameElement.str());

  return boost::shared_ptr<chemkit::Molecule>(molecule);
}

// --- Internal Methods ---------------------------------------------------- //
/// Returns the name of the molecules collection.
std::string MongoDatabase::moleculesCollectionName() const
{
  QSettings settings;

  QString collection = settings.value("collection", "chem").toString();

  return collection.toStdString() + ".molecules";
}

/// Creates a molecule ref using the object ID of \p obj.
MoleculeRef MongoDatabase::createMoleculeRefForBSONObj(const mongo::BSONObj &obj) const
{
  if (obj.isEmpty())
    return MoleculeRef();

  mongo::BSONElement idElement;
  bool ok = obj.getObjectID(idElement);
  if (!ok)
    return MoleculeRef();

  return MoleculeRef(idElement.OID());
}