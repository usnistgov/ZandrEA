// This file is in library EA of the ZandrEA (tm) open-source software development project for research
// in automated fault detection and diagnostics (AFDD) of the mechanical systems in commercial buildings.
// File origin: DAV at U.S. National Institute of Standards and Technology (NIST). See ZandrEA on GitHub.
/*
   Header declaring RAII wrapper classes needed for utilizing HDF5 generally throughout EA code
*/
//XXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

#ifndef HDF5PARTS_HPP
#define HDF5PARTS_HPP

#include "hdf5.h"

#include "diagnosticTypes.hpp"
#include <memory>
#include <functional>

namespace H5Kit {

//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// File classes

class CFileCreatedIffAbsent {    // File gets created if and only if not one already with given name

   public:

      CFileCreatedIffAbsent( const std::string& );
      ~CFileCreatedIffAbsent( void );

      bool WasNewEmptyKbaseMade( void ) const;
      bool WasPreexistingKbaseFound( void ) const;

      // Object checks if creation required then is destroyed.  Class not intended to pass an Id out.  

   private:

      hid_t    id;
      bool     namePreexistsAsHdf5;
      bool     nameMadeIntoNewEmptyHdf5;

      CFileCreatedIffAbsent( const CFileCreatedIffAbsent& ) = delete;
      CFileCreatedIffAbsent& operator=( const CFileCreatedIffAbsent& ) = delete;


};
 

class CFileOpenedReadOnly {

   public:

      CFileOpenedReadOnly( const std::string& );
      ~CFileOpenedReadOnly( void );
      CFileOpenedReadOnly( const CFileOpenedReadOnly& ) = delete;
      CFileOpenedReadOnly& operator=( const CFileOpenedReadOnly& ) = delete;

      hid_t SayId( void ) const;

   private:

      hid_t id;
};


class CFileOpenedReadWrite {

   public:

      CFileOpenedReadWrite( const std::string& );
      ~CFileOpenedReadWrite( void );
      CFileOpenedReadWrite( const CFileOpenedReadWrite& ) = delete;
      CFileOpenedReadWrite& operator=( const CFileOpenedReadWrite& ) = delete;

      hid_t SayId( void ) const;

   private:

      hid_t id;
};


//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Group classes

class CGroupCreated {

   public:

      CGroupCreated( hid_t, const std::string&);
      ~CGroupCreated( void );
      CGroupCreated( const CGroupCreated& ) = delete;
      CGroupCreated& operator=( const CGroupCreated& ) = delete;

      hid_t SayId( void ) const;

   private:

      hid_t id;
};


class CGroupOpened {

   public:

      CGroupOpened( hid_t, const std::string& );
      ~CGroupOpened( void );
      CGroupOpened( const CGroupOpened& ) = delete;
      CGroupOpened& operator=( const CGroupOpened& ) = delete;

      hid_t SayId( void ) const;

   private:

      hid_t id;
};


// Declare in H5Kit:: namespace an IteratedOp(), having H5G_iterate_t signature 
herr_t IteratedOp( hid_t, const char*, void* ); 


class CGroupIteratedOnSubgrps {

   public:

      CGroupIteratedOnSubgrps( hid_t, std::string&, const char );
      ~CGroupIteratedOnSubgrps( void );

      CGroupIteratedOnSubgrps( const CGroupIteratedOnSubgrps& ) = delete;
      CGroupIteratedOnSubgrps& operator=( const CGroupIteratedOnSubgrps& ) = delete;

      std::vector<std::string>   SayNamesFound( void );
      std::vector<Nzint_t>       SayIdsFound( void );
      int                        SayNumSubgrpsFound( void );


   private:

      std::vector<std::string>      namesExtracted;
      std::vector<Nzint_t>          idsExtracted;
      std::array<char,100>          charsFound;    // 100 chars allows up to 25 4-char subgrp names
      int                           numSubgrpsFound;
      size_t                        numNamesExtracted;
      size_t                        numIdsExtracted;
      const char                    axisExpected;

      int     ExtractNamesFromChars( void );
      int     ExtractIdsFromChars( void );

};


//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Dataset classes

class CDatasetCreated {

   public:

      CDatasetCreated( hid_t, const std::string&, size_t, size_t, int );
      ~CDatasetCreated( void );
      CDatasetCreated( const CDatasetCreated& ) = delete;
      CDatasetCreated& operator=( const CDatasetCreated& ) = delete;

      hid_t SayId( void ) const;

   private:

      hid_t id;
};


class CDatasetOpened {

   public:

      CDatasetOpened( hid_t, const std::string& );
      ~CDatasetOpened( void );
      CDatasetOpened( const CDatasetOpened& ) = delete;
      CDatasetOpened& operator=( const CDatasetOpened& ) = delete;

      hid_t SayId( void ) const;

   private:

      hid_t id;
};


class CDatasetMediator {

public:

   CDatasetMediator( Knode_t&, bool );
   ~CDatasetMediator( void );
   CDatasetMediator( const CDatasetMediator& ) = delete;
   CDatasetMediator& operator=( const CDatasetMediator& ) = delete;

   char        SetDatasetIdTo( hid_t );
   char        CopyDatasetToImage( void );
   char        CopyImageToDataset( void );
   char        FlushAfterWrite( void );
   
private:

   Knode_t&                   ImageRef;
   const hid_t                datatypeId;
   const hsize_t              mutualDims[2];
   const hid_t                dataspaceId;
   hid_t                      datasetId;
   herr_t                     hdfErr;
   const bool                 canOverwriteDatasets;
   bool                       datasetIdValid;      
};


//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// Attribute classes

class CAttributeCreatedAndSet {

public:

   CAttributeCreatedAndSet(   hid_t,
                              const std::string&,
                              const std::array<Nzint_t,3>& );

   ~CAttributeCreatedAndSet( void );
   CAttributeCreatedAndSet( const CAttributeCreatedAndSet& ) = delete;
   CAttributeCreatedAndSet& operator=( const CAttributeCreatedAndSet& ) = delete;

};


class CAttributeRead {

   public:

      CAttributeRead(   hid_t,
                        const std::string&,
                        std::array<Nzint_t,3>& );

      ~CAttributeRead( void );

      CAttributeRead(const CAttributeRead& ) = delete;
      CAttributeRead& operator=( const CAttributeRead& ) = delete;
};


//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
// C++ classes to mute/catch/handle errors generated within HDF5 C API

class CMuteHDF5ErrorHdlg {

   public:

      CMuteHDF5ErrorHdlg( void );

      ~CMuteHDF5ErrorHdlg( void );

      CMuteHDF5ErrorHdlg( const CMuteHDF5ErrorHdlg& ) = delete;
      CMuteHDF5ErrorHdlg& operator=( const CMuteHDF5ErrorHdlg& ) = delete;

      void     Unmute( void );  // Constructor mutes all errors, so if want unmute must call prior to destroy

   private:

      hid_t                errStackId;    // estack_id in HDF5 documentation
      H5E_auto2_t          errCallBk;     // func, function HDF5 to call upon API error;
      void*                callBkData;    // client_data, data to be passed to error callback

};


//VVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVVV
}  // end namespace

#endif

/* START FILE NOTES XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX

[1]   The following is already declared by the HDF5 C API in H5Lpublic.h (or its other headers):

      A namespace called "H5"

      A prototype for H5Literate/H5Literate_by_name() operator :
         typedef herr_t(*H5L_iterate_t)(hid_t group, const char *name, const H5L_info_t *info,
            void *op_data);

       An information struct for link (for H5Lget_info/H5Lget_info_by_idx) :
         typedef struct {
            H5L_type_t          type;           // Type of link                   
            hbool_t             corder_valid;   // Indicate if creation order is valid
            int64_t             corder;         // Creation order
            H5T_cset_t          cset;           // Character set of link name
            union {
               haddr_t         address;        // Address hard link points to
               size_t          val_size;       // Size of a soft link or UD link value
            } u;
         } H5L_info_t;

[2]   Misc. bones from trying to hook up group (link) iteration action:

      typedef std::function<herr_t( hid_t, const char*, const H5L_info_t*, void* )> H5L_iterate_t;

      herr_t (*H5L_iterate_t) (hid_t g_id, const char *name, const H5L_info_t *info, void *op_data);

      herr_t H5Literate(hid_t group_id, H5_index_t index_type, H5_iter_order_t order, hsize_t *idx, H5L_iterate_t op, void *op_data)

      herr_t file_info(hid_t loc_id, const char *name, void *opdata);

      idx = H5Giterate(file, "/", NULL, file_info, NULL);


//--------------------------------------------------------------------------------
XXX END FILE NOTES */


//END-OF-FILE ZZZZZ2ZZZZZZZZZ3ZZZZZZZZZ4ZZZZZZZZZ5ZZZZZZZZZ6ZZZZZZZZZ7ZZZZZZZZZ8ZZZZZZZZZ9ZZZZZZZZZCZZZZZ
