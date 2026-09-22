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

  // Username with no password and no port
  //
  {
    DatabaseUrl no_password_no_port("postgres://alice@localhost/mydb");

    assert(no_password_no_port.username == "alice");
    assert(no_password_no_port.password == "");
    assert(no_password_no_port.hostname == "localhost");
    assert(no_password_no_port.database_name == "mydb");
  }

  // no port, no slash, no database
  //
  {
    DatabaseUrl bare_host("postgres://alice@localhost");

    assert(bare_host.username == "alice");
    assert(bare_host.hostname == "localhost");
    assert(bare_host.port == 0);
    assert(bare_host.database_name == "");
  }

  // Trailing slash with no database name
  // 
  {
    DatabaseUrl trailing_slash("postgres://alice@localhost/");

    assert(trailing_slash.hostname == "localhost");
    assert(trailing_slash.database_name == "");
  }

  // Port present but nothing after it
  // 
  {
    DatabaseUrl port_only("postgres://alice@localhost:5432");

    assert(port_only.hostname == "localhost");
    assert(port_only.port == 5432);
    assert(port_only.database_name == "");
  }

  // Missing protocol
  //
  {
    DatabaseUrl no_protocol("alice:secret@localhost/mydb");

    assert(no_protocol.type == "");
  }

  // Does not crash when constructing from NULL strings
  //
  {
    const char* null_str = 0;
    try {
      DatabaseUrl url(null_str);
      assert(false); // should've thrown an exception
    } catch (const std::exception&) {
    }
  }

  return 0;
}
