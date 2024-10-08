#include <windows.h>

#include <string>
#include <iostream>
#include <strsafe.h>

#include <boost/regex.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>

#include "Source.h"

#include "pgconnection.h"


std::string m_dbhost = "localhost";
std::string m_dbport = "5432";
std::string m_dbname = "";
std::string m_dbuser = "";
std::string m_dbpass = "";

PGConnection conn;


PGConnection::~PGConnection()
{
    PQfinish(m_connection);
}


bool PGConnection::connection()
{
    m_connection = PQsetdbLogin(
        m_dbhost.c_str(), 
        m_dbport.c_str(), 
        nullptr, nullptr, 
        m_dbname.c_str(), 
        m_dbuser.c_str(), 
        m_dbpass.c_str());
    try
    {
        ConnStatusType st = PQstatus(m_connection);
        if(st != CONNECTION_OK && PQsetnonblocking(m_connection, 1) != 0)
        {
            connections = false;
            throw std::runtime_error(PQerrorMessage(m_connection));
        }
        PGresult* res = PGexec("set time zone 'Asia/Yekaterinburg';");
        if(PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            std::string errc = utf8_to_cp1251(PQresultErrorMessage(res));
            boost::replace_all(errc, "\n", "\r\n");
            SetWindowText(ExecuteOut, errc.c_str());
        }
        PQclear(res);

        connections = true;
    }
    catch(std::runtime_error rt)
    {
        MessageBox(NULL, rt.what(), "Error", 0);
    }
    catch(...)
    {
        MessageBox(NULL, "Неизвестная ошибка", "Error", 0);
    }
    return connections;
}

PGresult* PGConnection::PGexec(std::string std)
{
    return PQexec(m_connection, std.c_str());
}



std::string StrOut;
void testConnection(std::string exec)
{
    SetWindowText(ExecuteOut, "");

    std::ofstream f("command.txt", std::ios::binary | std::ios::out | std::ios::trunc);
    if(f.is_open())
    {
        f << exec;
        f.close();
    }

    std::string s= cp1251_to_utf8(exec);
    PGresult* res = conn.PGexec(s);

    std::map <int, int >count;

    //int COUNT = 15;
    ExecStatusType st = PQresultStatus(res);
    if(st == PGRES_TUPLES_OK && PQntuples(res))
    {
        std::stringstream streams1;
        std::stringstream streams2;
        int nLiness = PQntuples(res);
        streams1 << "Найдено " << nLiness << " записей\r\n\r\n";
        int nFields = PQnfields(res);

        for(int j = 0; j < nFields; j++)
        {
            std::string l =  utf8_to_cp1251(PQfname(res, j));
            count[j] = (std::max)(count[j], (int)l.length());
        }

        for(int i = 0; i < nLiness; i++)
        {
            for(int j = 0; j < nFields; j++)
            {
                
                std::string l = utf8_to_cp1251(PQgetvalue(res, i, j));
                count[j] = (std::max)(count[j], (int)l.length());
            }
        }

        streams1 << "| ";
        for(int j = 0; j < nFields; j++)
        {
            std::string d = utf8_to_cp1251(PQfname(res, j));
            streams1 << std::setw(count[j]) << d << " | ";
            streams2 << d << ";";
        }
        streams1 << "\r\n";
        streams2 << std::endl;

        streams1 << "|-";
        //streams << ;
        for(int j = 0; j < nFields; j++)
        {
            for(int n = 0; n < count[j] ; n++)
                streams1 << "-";
            streams1 << "-|-";
        }
        streams1 << "\r\n";

        for(int i = 0; i < nLiness; i++)
        {
            streams1 << "| ";
            for(int j = 0; j < nFields; j++)
            {
                std::string d = utf8_to_cp1251(PQgetvalue(res, i, j));
                streams1 << std::setw(count[j]) << d << " | ";
                streams2 << d << ";";
            }
            streams1 << "\r\n";
            streams2 << std::endl;
        }
        StrOut = streams2.str();
        SetWindowText(ExecuteOut, streams1.str().c_str());
    }
    else if(st == PGRES_FATAL_ERROR)
    {
        std::string errc = utf8_to_cp1251(PQresultErrorMessage(res));
        boost::replace_all(errc, "\n", "\r\n");
        SetWindowText(ExecuteOut, errc.c_str());
    }
    else
    {
        SetWindowText(ExecuteOut, "Not Found");
    }
    PQclear(res);
}
