#include <iostream>
#include <fstream>
#include <cpr/cpr.h>
#include <cpr/response.h>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>


struct UserInfo {
  std::string username;
  std::string global_name;
  std::string joined_date;
  std::string last_message_date;
};

struct MsgInfo {
  std::string username;
  std::string message_date;
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

  
  //const std::string CHANNEL_ID = "1307090118577750147";
  //const std::string CHANNEL_URL = "https://discord.com/api/v10/channels/" + CHANNEL_ID + "/messages?limit=100";

  const std::string BOT_TOKEN = temp_token;
  const std::string GUILD_ID = "1082338211973566605";
  const std::string GUILD_USER_URL = "https://discord.com/api/v10/guilds/" + GUILD_ID +"/members?limit=1000";
  const std::string GUILD_CHANNEL_URL = "https://discord.com/api/v10/guilds/" + GUILD_ID +"/channels";

  //Vector to store user_info struct elements
  std::vector<UserInfo> user_db;

  //Vector to store message info struct elements
  std::vector<MsgInfo> msg_db;

  //High engaged channels
  std::vector<std::string> high_engaged_channels = {
    //question-answer
    "1265420354696773683",
    //daily top 2
    "1299076561949032589",
    //daily journal
    "1097946965314113567",
    //active-support-qa
    "1097947193614270534"
  };


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
    std::ofstream outputFile("user_log.json");
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

    //Iterate through each channel an call API to get 100 messages
    for (const auto & channel : guild_channel_json) {
      if (!channel["id"].is_null()) {
        std::string channel_id = channel["id"];
        std::string limit = "100";
        int num_fetch = 1;
        if (std::find(high_engaged_channels.begin(), high_engaged_channels.end(), channel_id) != high_engaged_channels.end()) {
          num_fetch = 5;
        }
        std::string CHANNEL_MESSAGE_URL = "https://discord.com/api/v10/channels/" + channel_id + "/messages?limit=" + limit;
        
        for (int i = 0; i < num_fetch; i++ ) {
          cpr::Response channel_message_response = cpr::Get(
              cpr::Url{CHANNEL_MESSAGE_URL},
              cpr::Header{
                {"Authorization", "Bot " + BOT_TOKEN}
              }
            );
          
          if (channel_message_response.status_code == 200) {
            nlohmann::json channel_message_json = nlohmann::json::parse(channel_message_response.text);

            for (const auto & message : channel_message_json) {

              std::string record_usrname = "Unknown";
              if (message.contains("author") && message["author"].contains("username")) {
                record_usrname = message["author"]["username"];
              }

              std::string record_date = "Unknown";
              if (message.contains("timestamp")) {
                record_date = message["timestamp"];
              }
              std::string record_date_short = record_date.substr(0,10);

              struct MsgInfo new_record;
              new_record.username = record_usrname;
              new_record.message_date = record_date_short;

              //Perform a find on msg_db to look if there's a record with the same username exists
              auto it = std::find_if(msg_db.begin(), msg_db.end(), [&](const MsgInfo &record) {
                  return record.username == new_record.username;
              });

              if (it == msg_db.end()) {
                  // If no record with the same username exists, add it to the db
                  msg_db.push_back(new_record);
              }
            }

            // Open a file for writing
            std::ofstream outputFile("channel_log.json", std::ios::app | std::ios::out);
            if (!outputFile.is_open()) {
              std::cerr << "Error: Could not open the file for writing!" << std::endl;
              return 1;
            }

            // Write the JSON to the file in a formatted way
            outputFile << channel_message_json.dump(4); // 4-space indentation for pretty printing

            // Close the file
            outputFile.close();
          } else {
            std::cerr << "Error: HTTP "<< channel_message_response.status_code << "-" << channel_message_response.text << std::endl;
          }
        }
      }
    }
    
    std::ofstream outFile("message_log.txt", std::ios::app | std::ios::out);
    if (!outFile.is_open()) {
      std::cerr << "Error: Could not open the file" << std::endl;
      return 1;
    }
    for (const auto & element : msg_db) {
      std::cout << "username: " << element.username << " | Last_Message_Date: " << element.message_date << std::endl;
      std::string outStr = "username: " + element.username + " | last msg date: " + element.message_date + "\n";
      outFile << outStr;
    }
    //Close file
    outFile.close();

    // Open a file for writing
    std::ofstream outputFile("server_log.json");
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
  

  return 0;

}
