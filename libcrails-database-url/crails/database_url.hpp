#ifndef  DATABASE_URL_HPP
# define DATABASE_URL_HPP

# include <string_view>
# include <string>
# include <utility>

namespace Crails
{
  struct DatabaseUrl
  {
    DatabaseUrl() {}
    DatabaseUrl(const std::string_view);

    void initialize(const std::string_view url);
    std::string to_string() const;
    operator std::string() const { return to_string(); }

    std::string  type, hostname, username, password, database_name;
    unsigned int port;

  private:
    std::string_view substr(const std::string_view url, std::pair<int,int> range);
  };
}

#endif
