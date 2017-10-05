#ifndef _SAPPHIRE_DBCONNECTION_H
#define _SAPPHIRE_DBCONNECTION_H

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include "src/servers/Server_Common/Util/LockedWaitQueue.h"
#include <Server_Common/Util/LockedWaitQueue.h>
#include <boost/scoped_ptr.hpp>

namespace Mysql
{
   class Connection;
   class ResultSet;
   class PreparedResultSet;
   class PreparedStatement;
}

namespace Core
{
namespace Db
{
   class DatabaseWorker;
   class PreparedStatement;
   class Operation;
   class DbWorker;

   typedef boost::scoped_ptr< PreparedStatement > PreparedStmtScopedPtr;

   enum ConnectionFlags
   {
      CONNECTION_ASYNC = 0x1,
      CONNECTION_SYNCH = 0x2,
      CONNECTION_BOTH = CONNECTION_ASYNC | CONNECTION_SYNCH
   };

   struct ConnectionInfo
   {
      std::string user;
      std::string password;
      std::string database;
      std::string host;
      uint16_t port;
      uint8_t syncThreads;
      uint8_t asyncThreads;
   };

   typedef std::map< uint32_t, std::pair< std::string, ConnectionFlags > > PreparedStatementMap;

   class DbConnection
   {
   public:
      // Constructor for synchronous connections.
      DbConnection( ConnectionInfo& connInfo );
      // Constructor for asynchronous connections.
      DbConnection( Core::LockedWaitQueue< Operation* >* queue, ConnectionInfo& connInfo );
      virtual ~DbConnection();

      virtual uint32_t open();
      void close();

      bool prepareStatements();

      bool execute( const std::string& sql );
      bool execute( PreparedStatement* stmt );
      Mysql::ResultSet* query( const std::string& sql );
      Mysql::ResultSet* query( PreparedStatement* stmt );

      void beginTransaction();
      void rollbackTransaction();
      void commitTransaction();
      
      void ping();

      uint32_t getLastError();
      bool lockIfReady();

      void unlock();

      std::shared_ptr< Mysql::Connection > getConnection() { return m_pConnection; }
      Mysql::PreparedStatement* getPreparedStatement( uint32_t index );
      void prepareStatement( uint32_t index, const std::string& sql, ConnectionFlags flags );

      virtual void doPrepareStatements() = 0;

   protected:
      std::vector< std::unique_ptr< Mysql::PreparedStatement > > m_stmts;
      PreparedStatementMap m_queries;
      bool m_reconnecting;
      bool m_prepareError;

   private:
      LockedWaitQueue< Operation* >* m_queue;
      std::unique_ptr< DbWorker > m_worker;
      std::shared_ptr< Mysql::Connection > m_pConnection;
      ConnectionInfo& m_connectionInfo;
      ConnectionFlags m_connectionFlags;
      std::mutex m_mutex;

      DbConnection( DbConnection const& right ) = delete;
      DbConnection& operator=( DbConnection const& right ) = delete;
   };

}
}


#endif