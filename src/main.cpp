#include <cpr/response.h>
#include <iostream>
#include <fstream>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <string>


struct UserInfo {
  std::string username;
  std::string global_name;
  std::string joined_date;

};

int main (int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <bot-token.txt>" << std::endl;
    exit(-1);
  }

  std::ifstream bot_token_file(argv[1]);
  if (!bot_token_file.is_open()) {
    std::cerr << "Error: Cannot open file" << std::endl;
  }
  std::string temp_token; 
  std::getline(bot_token_file, temp_token);
  bot_token_file.close();

  const std::string BOT_TOKEN = temp_token;
  const std::string GUILD_ID = "1082338211973566605";
  
  const std::string CHANNEL_ID = "1307090118577750147";
  const std::string CHANNEL_URL = "https://discord.com/api/v10/channels/" + CHANNEL_ID + "/messages?limit=100";

  const std::string GUILD_USER_URL = "https://discord.com/api/v10/guilds/" + GUILD_ID +"/members?limit=1000";
  const std::string GUILD_CHANNEL_URL = "https://discord.com/api/v10/guilds/" + GUILD_ID +"/channels";

  //Vector to store user_info_t struct
  std::vector<UserInfo> user_db;





   //NOTE: === FETCH ALL CHANNEL LOG
  //Make GET request
  cpr::Response guild_channel_response = cpr::Get(
      cpr::Url{GUILD_CHANNEL_URL},
      cpr::Header{
        {"Authorization", "Bot " + BOT_TOKEN}
      }
  );

  //Check return Status and parse JSON 
  if (guild_channel_response.status_code == 200) {
    nlohmann::json guild_channel_json = nlohmann::json::parse(guild_channel_response.text);

    // Open a file for writing
    std::ofstream outputFile("server_channel_log.json");
    if (!outputFile.is_open()) {
      std::cerr << "Error: Could not open the file for writing!" << std::endl;
      return 1;
    }

    // Write the JSON to the file in a formatted way
    outputFile << guild_channel_json.dump(4); // 4-space indentation for pretty printing

    // Close the file
    outputFile.close();
  } else {
    std::cerr << "Error: HTTP " << guild_channel_response.status_code << "-" << guild_channel_response.text << std::endl;
  }
  

  //NOTE:: === GET MEMEBER Info // 
  //Make GET request
  cpr::Response response = cpr::Get(
      cpr::Url(GUILD_USER_URL),
      cpr::Header{{"Authorization","Bot " + BOT_TOKEN}}
      );

  //Check return Status and parse JSON 
  if (response.status_code == 200) {
    nlohmann::json json_response = nlohmann::json::parse(response.text);
    // Open a file for writing
    std::ofstream outputFile("output.json");
    if (!outputFile.is_open()) {
      std::cerr << "Error: Could not open the file for writing!" << std::endl;
      return 1;
    }

    // Write the JSON to the file in a formatted way
    outputFile << json_response.dump(4); // 4-space indentation for pretty printing

    // Close the file
    outputFile.close();

    //Interate and store user data to user_db
    for (const auto & member : json_response) {
      struct UserInfo new_user;
      std::string global_name = "Unknown";
      if (member.contains("user") && member["user"].contains("global_name") && !member["user"]["global_name"].is_null()) {
        global_name = member["user"]["global_name"].get<std::string>();
      }
      std::string username = "Unknown";
      if (member.contains("user") && member["user"].contains("username") && !member["user"]["username"].is_null()){
        username = member["user"]["username"].get<std::string>();
      }
      std::string join_date = "Unknown";
      if (member.contains("joined_at") && !member["joined_at"].is_null()){
        join_date = member["joined_at"].get<std::string>();
      }
      std::string join_date_short = join_date.substr(0,10);

      new_user.username = username;
      new_user.global_name = global_name;
      new_user.joined_date = join_date_short;
      user_db.push_back(new_user);

      //std::cout << "Name: " << global_name << " | Join date: " << join_date_short << std::endl; 
    }
  
  } else {
    std::cerr << "Error: HTTP " << response.status_code << "-" << response.text << std::endl;
  }

  for (int i = 0; static_cast<size_t>(i) < user_db.size(); i++) {
    std::cout << "username: " << user_db[i].username << " | Name: " << user_db[i].global_name << " | Join Date: " << user_db[i].joined_date << std::endl;
  }
  std::cout << "TOTAL : " << user_db.size() << std::endl;
  return 0;

}
