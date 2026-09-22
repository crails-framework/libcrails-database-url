#include <cstdlib>
#include <sstream>
#include "database_url.hpp"

using namespace std;
using namespace Crails;

static size_t advance_past(size_t pos, size_t delimiter_length)
{
  return pos == string_view::npos ? string_view::npos : pos + delimiter_length;
}

static pair<size_t, size_t> get_protocol_range(const string_view url)
{
  size_t end = url.find("://");

  return end == string_view::npos
    ? pair<size_t, size_t>{ 0, 0 }
    : pair<size_t, size_t>{ 0, end };
}

static pair<size_t, size_t> get_username_range(const string_view url)
{
  size_t start_name = advance_past(get_protocol_range(url).second, 3);
  size_t next_colon = url.find(':', start_name);
  size_t next_arobase = url.find('@', start_name);

  return { start_name, min(next_colon, next_arobase) };
}

static pair<size_t, size_t> get_password_range(const string_view url)
{
  size_t separator = get_username_range(url).second;
  size_t start_password = advance_past(separator, 1);

  if (separator >= url.length() || url[separator] == '@')
    return { 0, 0 };
  return { start_password, url.find('@', start_password) };
}

static pair<size_t, size_t> get_hostname_range(const string_view url)
{
  size_t credentials_end = url.find('@');
  size_t start_hostname = credentials_end != string_view::npos
    ? credentials_end + 1
    : advance_past(get_protocol_range(url).second, 3);
  size_t next_colon = url.find(':', start_hostname);
  size_t next_slash = url.find('/', start_hostname);

  return { start_hostname, min(next_colon, next_slash) };
}

static pair<size_t, size_t> get_port_range(const string_view url)
{
  size_t separator = get_hostname_range(url).second;
  size_t start_port = advance_past(separator, 1);

  if (separator >= url.length() || url[separator] != ':')
    return { separator, separator };
  return { start_port, url.find('/', start_port) };
}

static pair<size_t, size_t> get_database_name_range(const string_view url)
{
  size_t start_database_name = advance_past(get_port_range(url).second, 1);

  if (start_database_name >= url.length())
    return { 0, 0 };
  return { start_database_name, url.length() };
}

static string redact_credentials(const string_view url)
{
  size_t protocol_end = url.find("://");
  size_t at           = url.find('@');

  if (protocol_end == string_view::npos || at == string_view::npos || at < protocol_end)
    return string(url); // no credentials to hide
  return string(url.substr(0, protocol_end + 3)) + "***@" + string(url.substr(at + 1));
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
        string("Failed to read database url string ") + redact_credentials(url) + ": " + e.what()
      );
    }
  }
}

string DatabaseUrl::to_redacted_string() const
{
  return redact_credentials(to_string());
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

string_view DatabaseUrl::substr(const string_view url, pair<size_t, size_t> range)
{
  if (range.first == string_view::npos || range.first == range.second)
    return {};
  if (range.second == string_view::npos)
    return url.substr(range.first); // no closing delimiter found: goes to the end
  return url.substr(range.first, range.second - range.first);
}
