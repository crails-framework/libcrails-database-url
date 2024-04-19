#include <cstdlib>
#include <sstream>
#include "database_url.hpp"

using namespace std;
using namespace Crails;

static pair<int, int> get_protocol_range(const string_view url)
{
  return { 0, url.find("://") };
}

static pair<int, int> get_username_range(const string_view url)
{
  int start_name = get_protocol_range(url).second + 3;
  int next_colon = url.find(':', start_name);
  int next_arobase = url.find('@', start_name);

  return { start_name, min(next_colon, next_arobase) };
}

static pair<int, int> get_password_range(const string_view url)
{
  int separator = get_username_range(url).second;
  int start_password = separator + 1;

  if (url[separator] == '@')
    return { 0, 0 };
  return { start_password, url.find("@", start_password) };
}

static pair<int, int> get_hostname_range(const string_view url)
{
  int credentials_end = url.find('@');
  int start_hostname = credentials_end != string::npos
    ? credentials_end + 1
    : get_protocol_range(url).second + 3;
  int next_colon = url.find(':', start_hostname);
  int next_slash = url.find('/', start_hostname);

  next_colon = next_colon == string::npos ? url.length() : next_colon;
  next_slash = next_slash == string::npos ? url.length() : next_slash;
  return { start_hostname, min(next_colon, next_slash) };
}

static pair<int, int> get_port_range(const string_view url)
{
  int separator = get_hostname_range(url).second;
  int start_port = separator + 1;
  int end_port = url.find('/', start_port);

  if (url[separator] != ':')
    return { separator, separator };
  if (end_port == string::npos)
    return { start_port, url.length() };
  return { start_port, end_port };
}

static pair<int, int> get_database_name_range(const string_view url)
{
  int start_database_name = get_port_range(url).second + 1;

  if (start_database_name >= url.length())
    return { 0, 0 };
  return { start_database_name, url.length() };
}

DatabaseUrl::DatabaseUrl(const char* url)
{
  if (!url)
    throw runtime_error("cannot initialize DatabaseUrl from a NULL string");
  initialize(url);
}

DatabaseUrl::DatabaseUrl(const string_view url)
{
  initialize(url);
}

void DatabaseUrl::initialize(const string_view url)
{
  if (url.length() > 0)
  {
    try
    {
      bool has_username = url.find('@') != string::npos;

      type     = string(substr(url, get_protocol_range(url)));
      hostname = string(substr(url, get_hostname_range(url)));
      if (has_username)
      {
        username = string(substr(url, get_username_range(url)));
        password = string(substr(url, get_password_range(url)));
      }
      port     = atoi(string(substr(url, get_port_range(url))).c_str());
      database_name = string(substr(url, get_database_name_range(url)));
    }
    catch (exception& e)
    {
      throw runtime_error(
        string("Failed to read database url string ") + string(url) + ": " + e.what()
      );
    }
  }
}

string DatabaseUrl::to_string() const
{
  stringstream stream;

  stream << type << "://";
  if (username.length())
  {
    stream << username;
    if (password.length())
      stream << ':' << password;
    stream << '@';
  }
  stream << hostname;
  if (port != 0)
    stream << ':' << port;
  if (database_name.length())
    stream << '/' << database_name;
  return stream.str();
}

string_view DatabaseUrl::substr(const string_view url, pair<int, int> range)
{
  if (range.first < 0 || range.first == range.second)
    return {};
  return url.substr(range.first, range.second - range.first);
}
