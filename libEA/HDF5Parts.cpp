// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
Implementations of RAII classes needed to implement HDF5. 
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#pragma warning(disable:4996)    // Was needed for MSVC compile, TBD if still relevant under GCC

#include "HDF5Parts.hpp"

#include <array>
#include <cerrno>
#include <cstring>
#include <sstream>

#include "portability.hpp"


//VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
// File created 


H5Kit::CFileCreatedIffAbsent::CFileCreatedIffAbsent( const std::string& name )
                                                     :   id (-1),
                                                         namePreexistsAsHdf5 (false),
                                                         nameMadeIntoNewEmptyHdf5 (false) {

   errno = 0;
   // htri_t: <0 = fail or filename doesn't exist, 0 = filename exists but not HDF5, >0 = true
   htri_t nameCheck = H5Fis_hdf5( name.c_str() );
   
   if ( nameCheck < 1 ) {

      id = H5Fcreate( name.c_str(), H5F_ACC_EXCL, H5P_DEFAULT, H5P_DEFAULT );

      if (id < 0) {
         std::ostringstream squawk;
         // write alert collecting any C-side errno and throw to ext. generator
         squawk << "Failed creating new HDF file " << name << ": " << std::strerror(errno);
         throw std::runtime_error( squawk.str() );
      }
      nameMadeIntoNewEmptyHdf5 = true;
   }
   else {
      namePreexistsAsHdf5 = true;
   }
}


H5Kit::CFileCreatedIffAbsent::~CFileCreatedIffAbsent( void ) {

   if (id > -1) { H5Fclose( id ); } // No new HDF5 file was opened if id = -1
}


bool H5Kit::CFileCreatedIffAbsent::WasNewEmptyKbaseMade( void ) const {

   return nameMadeIntoNewEmptyHdf5;
}


bool H5Kit::CFileCreatedIffAbsent::WasPreexistingKbaseFound( void ) const {

   return namePreexistsAsHdf5;
}


//VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
// File opened

H5Kit::CFileOpenedReadOnly::CFileOpenedReadOnly( const std::string& name ) : id (-1) {

   errno = 0;
   id = H5Fopen( name.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT );
   if ( id < 0 ) {
      std::ostringstream squawk;
      squawk << "Cannot RO open HDF file " << name << ": " << std::strerror( errno );
      throw std::runtime_error( squawk.str() );
   }
}


H5Kit::CFileOpenedReadOnly::~CFileOpenedReadOnly( void ) {

   if (id > -1) { H5Fclose( id ); }
}


hid_t H5Kit::CFileOpenedReadOnly::SayId( void ) const { return id; }


H5Kit::CFileOpenedReadWrite::CFileOpenedReadWrite(const std::string& fileName) : id (-1) {

   errno = 0;
   id = H5Fopen(fileName.c_str(), H5F_ACC_RDWR, H5P_DEFAULT);
   if ( id < 0 ) {
      std::ostringstream squawk;
      squawk << "Cannot RW open HDF file " << fileName << ": " << std::strerror(errno);
      throw std::runtime_error( squawk.str() );
   }
}


H5Kit::CFileOpenedReadWrite::~CFileOpenedReadWrite(void) {

   if (id > -1) { H5Fclose( id ); }
}


hid_t H5Kit::CFileOpenedReadWrite::SayId( void ) const { return id; }


//VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
// Group created

H5Kit::CGroupCreated::CGroupCreated( hid_t locationId, const std::string& name ) : id (-1) {

   errno = 0;
   id = H5Gcreate2(  locationId,
                     name.c_str(),
                     H5P_DEFAULT,
                     H5P_DEFAULT,
                     H5P_DEFAULT );

   if (id < 0) {
      std::ostringstream squawk;
      squawk << "Cannot create HDF group " << name << ": " << std::strerror( errno );
      throw std::runtime_error( squawk.str() );
   }
}


H5Kit::CGroupCreated::~CGroupCreated( void ) {

   if (id > -1) { H5Gclose( id ); }
}


hid_t H5Kit::CGroupCreated::SayId( void ) const { return id; }


//VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
// Group opened


H5Kit::CGroupOpened::CGroupOpened( hid_t locationId, const std::string& grpName ) : id (-1) {

   errno = 0;
   id = H5Gopen2( locationId, grpName.c_str(), H5P_DEFAULT );
   if (id < 0) {
      std::ostringstream squawk;
      squawk << "Cannot open HDF group " << grpName << ": " << std::strerror( errno );
      throw std::runtime_error( squawk.str() );
   }
}


H5Kit::CGroupOpened::~CGroupOpened(void) {

   if (id > -1) { H5Gclose( id ); }
}


hid_t H5Kit::CGroupOpened::SayId( void ) const { return id; }


//VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
// Group iterated

int H5Kit::CGroupIteratedOnSubgrps::ExtractNamesFromChars(void) {

   int reply = -1;
   std::string nameToBuild;
   namesExtracted.clear();

   if (charsFound.at(0) != '\0') {

      auto p_char = charsFound.begin();
      while (p_char < charsFound.end()) {

         if (*p_char != '\0') {

            if (*p_char == axisExpected) {
               nameToBuild.clear();

               // name = 'R', 'H', or 'E', followed by 3-digit zero-padded number, so take 4 "laps"
               for (size_t laps = 0; laps<4; ++laps) {
                  nameToBuild.push_back(*p_char);
                  ++p_char;
               }
               namesExtracted.push_back(nameToBuild);
               ++numNamesExtracted;
            }
         }
         else {
            return reply;
         }
      }
      reply = 0;
   }
   return reply;
}


int H5Kit::CGroupIteratedOnSubgrps::ExtractIdsFromChars(void) {

   int reply = -1;
   Nzint_t idToBuild(0);
   idsExtracted.clear();

   /*
   charsFound a fixed 100-char array holding one c-string consisting of a variable number of 'name
   cycles' having an 'R', 'H', or 'E' (the axis), followed by a 3-digit zero-padded number
   (the ID to be extracted). After final cycle, c-string terminates in an ASCII NUL char ( '\0' ).
   */

   if (charsFound.at(0) != '\0') {           // Any subgroup names found at all?

      auto p_char = charsFound.begin();
      while (p_char < charsFound.end()) {

         if (*p_char != '\0') {              // Not at end of meaningful content in charsFound?

            if (*p_char == axisExpected) {   // true indicates another name cycle beginning
               idToBuild = 0;               // clear accumulator for new ID number
               ++p_char;                     // advance pointer to 100's digit
               idToBuild += ( ((*p_char) - 48) * 100 );
               ++p_char;                     // advance pointer to 10's digit
               idToBuild += ( ((*p_char) - 48) * 10 );
               ++p_char;                     // advance pointer to 1's digit
               idToBuild += ( ((*p_char) - 48) * 1 );

               idsExtracted.push_back(idToBuild);
               ++numIdsExtracted;
               ++p_char;                     // advance pointer past ID number just extracted
            }
            reply = 0;
         }
         else {
            return reply;
         }
      }
   }
   return reply;
}


herr_t H5Kit::IteratedOp( hid_t groupId, const char* nameCurrObjectInGrp, void* passStuffOut ) {

   // HDF5 requires this signature of iterated operator: H5G_iterate_t( hid_t, const char*, void* )
   // Iterated op must keep returning 0 for iterations to continue to end of object names under groupId

   // C-style grp names: (one letter, R, H, or E) + (3 digit ID number, each 0 thru 9) + NUL = 5 chars
   std::array<char,5> currName;
   currName.fill('\0');

   // cstring function args are dest ptr first, then const source ptr
   std::memcpy( currName.data(), nameCurrObjectInGrp, 5 );  // 4 name chars to dest, 5th is the NUL
   std::strncat( (char*)passStuffOut, currName.data(), 5 ); // size here must = namesFound.size()
   return 0;
}


H5Kit::CGroupIteratedOnSubgrps::CGroupIteratedOnSubgrps( hid_t groupId,
                                                         std::string& groupName,
                                                         const char arg )
                                                         :  namesExtracted(0),
                                                            idsExtracted(0),
                                                            charsFound(),
                                                            numSubgrpsFound (0),
                                                            numIdsExtracted (0),
                                                            numNamesExtracted (0),
                                                            axisExpected (arg) {

   errno = 0;
   herr_t hdfErr = H5Giterate(   groupId,                   // hid_t loc_id (can be of a group or file)
                                 groupName.c_str(),         // const char* name (of grp/file iterated)
                                 nullptr,                   // int* idx
                                 &IteratedOp,               // H5G_iterate_t operator
                                 (void*)charsFound.data()   // void* operator_data
   );

   if (hdfErr < 0) {
      std::ostringstream squawk;
      squawk << "Cannot iterate HDF group " << groupName << ": " << std::strerror(errno);
      throw std::runtime_error( squawk.str() );
   }

   ExtractNamesFromChars();
   ExtractIdsFromChars();
   numSubgrpsFound = (int) numNamesExtracted;

   if ( !(numNamesExtracted == numIdsExtracted) ) {
      namesExtracted.clear();
      idsExtracted.clear();
      numSubgrpsFound = -1;
   }
}


H5Kit::CGroupIteratedOnSubgrps::~CGroupIteratedOnSubgrps( void ) { /* empty, nothing to release */ }


std::vector<std::string> H5Kit::CGroupIteratedOnSubgrps::SayNamesFound( void ) { return namesExtracted; }


std::vector<Nzint_t> H5Kit::CGroupIteratedOnSubgrps::SayIdsFound( void ) { return idsExtracted; }


int H5Kit::CGroupIteratedOnSubgrps::SayNumSubgrpsFound( void ) { return numSubgrpsFound; }


//VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
// Dataset created 

H5Kit::CDatasetCreated::CDatasetCreated(  hid_t locationId,  // P.B.V. by RAII obj, no H5*close() here
                                          const std::string& name,
                                          size_t numRows,
                                          size_t numCols,
                                          int elementType )
                                          :  id (-1) {

   errno = 0;
   const hid_t datatypeId = H5Tcopy(H5T_NATIVE_LLONG);
   const std::array<hsize_t, 2> dims = { numRows, numCols };
   const hid_t dataspaceId = H5Screate_simple(2, dims.data(), NULL);

                                          // args per HDF5 C API documentation ("PL" = property list):
   id = H5Dcreate2(  locationId,              //  hid_t loc_id (location ID, a file or grp in a file)
                     name.c_str(),           //  const char *name (for the new dataset)
                     datatypeId,             //  hid_t dtype_id
                     dataspaceId,            //  hid_t space_id,
                     H5P_DEFAULT,            //  hid_t lcpl_id (link creation PL)
                     H5P_DEFAULT,            //  hid_t dcpl_id (dataset creation PL)
                     H5P_DEFAULT );          //  hid_t dapl_id (dataset access PL)

   H5Sclose(dataspaceId);
   H5Tclose(datatypeId);

   if ( (errno != 0) || (id < 0) ) {
      std::ostringstream squawk;
      squawk << "Cannot create HDF dataset " << name << ": " << std::strerror( errno );
      throw std::runtime_error( squawk.str() );
   }
}


H5Kit::CDatasetCreated::~CDatasetCreated( void ) {

   if (id > -1) { H5Dclose( id ); }

}


hid_t H5Kit::CDatasetCreated::SayId( void ) const { return id; }


//VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
// Open dataset


H5Kit::CDatasetOpened::CDatasetOpened( hid_t locationId, const std::string& datasetName ) : id (-1) {

   errno = 0;
   id = H5Dopen2( locationId, datasetName.c_str(), H5P_DEFAULT );
   if (id < 0) {
      std::ostringstream squawk;
      squawk << "Cannot open HDF dataset " << datasetName << ": " << std::strerror( errno );
      throw std::runtime_error( squawk.str() );
   }
}


H5Kit::CDatasetOpened::~CDatasetOpened( void ) {

   if (id > -1) { H5Dclose(id); }
}


hid_t H5Kit::CDatasetOpened::SayId( void ) const { return id; }


//VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
// Access datasets 


H5Kit::CDatasetMediator::CDatasetMediator( Knode_t& arg0,
                                           bool arg1 )    // allow overwriting of datasets
                                 :  ImageRef (arg0),
                                    datatypeId ( H5Tcopy(H5T_NATIVE_LLONG) ),
                                    mutualDims {3,2},  // C++11 uniform initializing syntax allows this
                                    dataspaceId ( H5Screate_simple(2, mutualDims, NULL) ),
                                    datasetId (-1),
                                    hdfErr (-1),
                                    canOverwriteDatasets (arg1),
                                    datasetIdValid (false) {

   if ( (datatypeId < 0) || (dataspaceId < 0) ) {
      std::ostringstream squawk;
      squawk << "Cannot create mediator to HDF :" << std::strerror(errno);
      throw std::runtime_error( squawk.str() );
   }

}


H5Kit::CDatasetMediator::~CDatasetMediator( void ) {

   H5Sclose( dataspaceId );
   H5Tclose( datatypeId );
}


char H5Kit::CDatasetMediator::CopyDatasetToImage( void ) {

   if ( !datasetIdValid ) { return 'x'; }
                                             // args per HDF5 C API documentation:
   hdfErr = H5Dread( datasetId,           // hid_t dataset_id
                     datatypeId,          // hid_t mem_type_id
                     dataspaceId,         // hid_t mem_space_id
                     H5S_ALL,             // hid_t file_space_id
                     H5P_DEFAULT,         // hid_t xfer_plist_id
                     ImageRef.data() );   // void* buf (read modifies buf)

   return ( (hdfErr < 0) ? 'f' : 'v' );
}


char H5Kit::CDatasetMediator::CopyImageToDataset( void ) {

   if (canOverwriteDatasets && datasetIdValid) {
                                                   // args per HDF5 C API documentation:
      hdfErr = H5Dwrite(   datasetId,           // hid_t dataset_id
                           datatypeId,          // hid_t mem_type_id
                           dataspaceId,         // hid_t mem_space_id
                           H5S_ALL,             // hid_t file_space_id
                           H5P_DEFAULT,         // hid_t xfer_plist_id
                           ImageRef.data() );   // const void* buf (write modifies dataset)

      return ( (hdfErr < 0) ? 'f' : 'v' );
   }
   else { return 'x'; }
}


char H5Kit::CDatasetMediator::FlushAfterWrite( void ) {

   if (canOverwriteDatasets && datasetIdValid) {
                                                // args per HDF5 C API documentation:
      hdfErr = H5Dflush( datasetId );           // hid_t dataset_id
      return ( (hdfErr < 0) ? 'f' : 'v' );
   }
   else { return 'x'; }
}

char H5Kit::CDatasetMediator::SetDatasetIdTo( hid_t arg ) {

   datasetId = arg;
   datasetIdValid = ( (datasetId < 0) ? false : true );
   return ( datasetIdValid ? 'v' : 'f' );
}


//VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
// Create, set value, and close an attribute via a single class 

H5Kit::CAttributeCreatedAndSet::CAttributeCreatedAndSet( hid_t locationId,
                                                         const std::string& name,
                                                         const std::array<Nzint_t,3>& setValue ) {
   errno = 0;
   const std::array<hsize_t, 2> dims = { 1, 3 };
   const hid_t typeId = H5Tcopy( H5T_NATIVE_UINT );     // presumed HDF5 match to Nzint_t in EA
   const hid_t spaceId = H5Scopy(H5Screate_simple(2, dims.data(), NULL));

   hid_t id = H5Acreate2(  locationId,
                           name.c_str(),
                           typeId,
                           spaceId,
                           H5P_DEFAULT,
                           H5P_DEFAULT);

   herr_t hdfErr = H5Awrite(  id,                  // hid_t attribute_id
                              typeId,              // hid_t mem_type_id
                              setValue.data() );   // const void* buf

   if ( (id < 0) || (hdfErr < 0) ) {
      std::ostringstream squawk;
      squawk << "Cannot create HDF attribute " << name << ": " << std::strerror(errno);
      throw std::runtime_error(squawk.str());
   }
   H5Sclose(spaceId);
   H5Tclose(typeId);
   H5Aclose(id);
}


H5Kit::CAttributeCreatedAndSet::~CAttributeCreatedAndSet( void ) {  /* empty */ }


//VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
// Open, read, and close attribute via a single class

H5Kit::CAttributeRead::CAttributeRead( hid_t locationId,
                                       const std::string& attrName,
                                       std::array<Nzint_t,3>& getValue ) {

   errno = 0;
   hid_t id = H5Aopen( locationId, attrName.c_str(), H5P_DEFAULT );
   const hid_t typeId = H5Tcopy(H5T_NATIVE_UINT);     // presumed HDF5 match to Nzint_t in EA
   herr_t hdfErr = H5Aread(   id,                  // hid_t attribute_id
                              typeId,              // hid_t mem_type_id
                              getValue.data() );   // void* buf 

   if ((id < 0) || (hdfErr < 0)) {
      std::ostringstream squawk;
      squawk << "Cannot open HDF attribute " << attrName << " on object ID "
         << std::to_string(locationId) << ": " << std::strerror(errno);
      throw std::runtime_error(squawk.str());
   }
   H5Tclose(typeId);
   H5Aclose(id);
}


H5Kit::CAttributeRead::~CAttributeRead( void ) {  /* empty */ }


//VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
// Classes for C++ client app to mute/catch/handle in any error generated within HDF5 C API


H5Kit::CMuteHDF5ErrorHdlg::CMuteHDF5ErrorHdlg( void ) :  errStackId (H5E_DEFAULT),
                                                         errCallBk (NULL),
                                                         callBkData (NULL) {

   const herr_t hdfErr = H5Eset_auto2( H5E_DEFAULT,   // errStackId
                                       NULL,          // errCallBk (NULL here turns HDF5 errors off)
                                       NULL );        // callBkData
}


H5Kit::CMuteHDF5ErrorHdlg::~CMuteHDF5ErrorHdlg( void ) {  /* empty */ }
 

void H5Kit::CMuteHDF5ErrorHdlg::Unmute( void ) {

   /* Constructor mutes all errors, so if want unmute must call Unmute() prior to destroying object
      TBD to implement this, requires saving HDF5 error handling states prior to mute, using
      herr_t H5Eget_auto2( hid_t estack_id, H5E_auto2_t * func, void **client_data )
      [don't get reasoning on ptr-to-ptr use]
   */
   return;
}

//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZ
