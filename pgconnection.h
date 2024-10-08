#pragma once
#ifndef PGCONNECTION_H
#define PGCONNECTION_H

#include <memory>
#include <mutex>
#include <libpq-fe.h>


#pragma comment(lib, "libpq.lib")

class PGConnection
{
public:
    PGconn* m_connection;
    bool connections = false;
    PGConnection() { };
    ~PGConnection();
    bool connection();
    PGresult* PGexec(std::string std);

private:
    //void establish_connection();
};

void testConnection(std::string exec);


#endif //PGCONNECTION_H