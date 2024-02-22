#include <sstream>
#include <stdexcept>
#include <crails/database_url.hpp>
#include <iostream>

#undef NDEBUG
#include <cassert>

int main ()
{
  using namespace std;
  using namespace Crails;

  // Basics.
  //
  {
    DatabaseUrl basic("postgres://192.168.0.42:5432/db_name");

    assert(basic.type == "postgres");
    assert(basic.hostname == "192.168.0.42");
    assert(basic.port == 5432);
    assert(basic.database_name == "db_name");
  }

  // Default port.
  //
  {
    DatabaseUrl basic2("postgres://192.168.0.43/db_name");

    assert(basic2.hostname == "192.168.0.43");
    assert(basic2.port == 0);
    assert(basic2.database_name == "db_name");
  }

  // With credentials.
  //
  {
    DatabaseUrl creds("mongodb://titi:tutu@1.2.3.4:1234");

    assert(creds.type == "mongodb");
    assert(creds.hostname == "1.2.3.4");
    assert(creds.port == 1234);
    assert(creds.database_name == "");
    assert(creds.username == "titi");
    assert(creds.password == "tutu");
  }

  // To string
  //
  {
    DatabaseUrl url;

    url.type = "redis";
    url.hostname = "localhost";
    url.port = 8787;
    assert(url.to_string() == "redis://localhost:8787");
    url.username = "roger";
    assert(url.to_string() == "redis://roger@localhost:8787");
    url.password = "paswd";
    assert(url.to_string() == "redis://roger:paswd@localhost:8787");
    url.database_name = "lenom";
    assert(url.to_string() == "redis://roger:paswd@localhost:8787/lenom");
    url.port = 0;
    assert(url.to_string() == "redis://roger:paswd@localhost/lenom");
  }

  return 0;
}
