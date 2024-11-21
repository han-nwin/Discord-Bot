#include <iomanip>
#include <iostream>
#include <fstream>
#include <cpr/cpr.h>
#include <cpr/response.h>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <sstream>
#include <iomanip>

struct UserInfo {
  std::string username = "NOT FOUND";
  std::string global_name = "NOT FOUND";
  std::string joined_date = "1970-01-01"; //Default to Unix epoch
  //message array: each pair has the first one as key
  std::vector<std::pair<std::string, std::string>> message_data = {};
};

int getInactiveDate (const std::string & last_date_str) {
  //Parse the input string into time point object
  std::tm tm = {};
  std::istringstream ss(last_date_str);
  ss >> std::get_time(&tm, "%Y-%m-%d");

  if (ss.fail()) {
    std::cerr << "Failed parsing date string" << std::endl;
    exit(1);
  }

  // Convert parsed date to time_point
  auto givenDate = std::chrono::system_clock::from_time_t(std::mktime(&tm));

  // Get today's date
  auto today = std::chrono::system_clock::now();
  auto todayDate = std::chrono::floor<std::chrono::days>(today); // Ignore time portion

  // Calculate the duration between the two dates
  auto duration = std::chrono::duration_cast<std::chrono::hours>(todayDate - givenDate).count();
  int days = duration / 24; // Convert hours to days

  return days;

}


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

  //Hashmap to store user_info struct elements with username as key
  std::unordered_map<std::string, UserInfo> user_db;


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
      
      //If found new user add to hashmap
      if (user_db.find(username) == user_db.end()) {
        user_db.insert({username, new_user});
      }
      //PRINT TEST
      for (const auto & [username, user_info] : user_db) {
        std::cout << "Username: " << user_info.username << "\n";
        std::cout << "Global Name: " << user_info.global_name << "\n";
        std::cout << "Joined Date: " << user_info.joined_date << "\n";
        std::cout << "Message Data:\n";
        for (const auto & message : user_info.message_data) {
            std::cout << "  - " << message.first << " | " << message.second << "\n";
        }
        std::cout << "------------------------\n";
      }
      std::cout << "TOTAL : " << user_db.size() << std::endl;
    }
  
  } else {
    std::cerr << "Error: HTTP " << response.status_code << "-" << response.text << std::endl;
  }





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
        std::string last_message_id = "";
        int num_fetch = 1;
        if (std::find(high_engaged_channels.begin(), high_engaged_channels.end(), channel_id) != high_engaged_channels.end()) {
          num_fetch = 5;
        }

        //base url
        std::string CHANNEL_MESSAGE_URL = "https://discord.com/api/v10/channels/" + channel_id + "/messages?limit=" + limit;
        
        for (int i = 0; i < num_fetch; i++ ) {

          //Update the link
          if (last_message_id != "") {
            CHANNEL_MESSAGE_URL += "&before=" + last_message_id;
          }

          //Make API Call
          cpr::Response channel_message_response = cpr::Get(
              cpr::Url{CHANNEL_MESSAGE_URL},
              cpr::Header{
                {"Authorization", "Bot " + BOT_TOKEN}
              }
            );
          
          //Processing each call
          if (channel_message_response.status_code == 200) {
            //Parse response to text
            nlohmann::json channel_message_json = nlohmann::json::parse(channel_message_response.text);

            //store last_message_id
            if (!channel_message_json.empty()) {
              last_message_id = channel_message_json.back()["id"];
            }


            for (const auto & message : channel_message_json) {

              std::string record_usrname = "Unknown";
              if (message.contains("author") && message["author"].contains("username")) {
                record_usrname = message["author"]["username"];
              }

              std::string record_date_long = "Unknown";
              if (message.contains("timestamp")) {
                record_date_long = message["timestamp"];
              }
             // std::string record_date_short = record_date.substr(0,10);

              std::string record_content = "Unknown";
              if (message.contains("content")) {
                record_content = message["content"];
              }
              
              //Found new user but this should not happen
              if (user_db.find(record_usrname) == user_db.end()) {

                struct UserInfo new_user;
                new_user.username = record_usrname;
               // new_user.global_name = record_glob_name;
                new_user.message_data.push_back({record_date_long, record_content});
                //add new user into db
                user_db.insert({record_usrname, new_user});

              } else { //user is already in db
                user_db[record_usrname].message_data.push_back({record_date_long,record_content}); //Append more dates
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
    

    // Print log
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


  //PRINT TEST
  for (const auto & [username, user_info] : user_db) {
    std::cout << "Username: " << user_info.username << "\n";
    std::cout << "Global Name: " << user_info.global_name << "\n";
    std::cout << "Joined Date: " << user_info.joined_date << "\n";
    std::cout << "Message Dates:\n";
    for (const auto& message : user_info.message_data) {
        std::cout << "  - " << message.first << " | " << message.second << "\n";
    }
    std::cout << "------------------------\n";
  }  

  //NOTE:Print result to Text file
  std::ofstream textFile("final_log.txt");
  if (!textFile.is_open()) {
    std::cerr << "Error: Could not open final_log.txt" << std::endl;
  }

  for (const auto & [username, user_info] : user_db) {
    textFile << "Username: " << user_info.username << "\n";
    textFile << "Global Name: " << user_info.global_name << "\n";
    textFile << "Joined Date: " << user_info.joined_date << "\n";
    textFile << "Message Dates:\n";
    for (const auto& message : user_info.message_data) {
        textFile << "  - " << message.first.substr(0,10) << " | " << message.second << "\n";
    }
    textFile << "------------------------\n";
  }
  //close text file
  textFile.close();

  //NOTE: EXPORT Result to CSV file
  std::ofstream outFile("final_log.csv");
  if (!outFile.is_open()) {
    std::cerr << "Error: Could not open the file" << std::endl;
    return 1;
  }

  //CSV Header
  outFile << "User Name,Global Name,Joined Date,Latest Message Date,Message Content,Status(last 30days)\n";

  for (auto& [username, user_info] : user_db) {
    // Write user details and the latest message date
    outFile << user_info.username << ","
            << user_info.global_name << ","
            << user_info.joined_date << ",";
    
    // Sort the message_data vector by date
    std::sort(user_info.message_data.begin(), user_info.message_data.end(), 
              [](const auto & a, const auto & b) { return a.first < b.first;});//NOTE: Learn Lambda function here

  /**
    struct {
      bool operator() (const std::pair<std::string, std::string> & a, const std::pair<std::string, std::string> & b) {
        return a.first < b.first;
      }
    } customLess;
    std::sort(user_info.message_data.begin(), user_info.message_data.end(), customLess); //NOTE: Comparision using struct 
    */

    if (!user_info.message_data.empty()) {
      //Check active date. If inactive more than 30 day -> Mark inactive
      int inactive_date = getInactiveDate( user_info.message_data.back().first.substr(0,10) );
      std::string active_status = "Inactive";
      if (inactive_date < 30) {
        active_status = "Active";
      }
      // Write the latest message date
      outFile << user_info.message_data.back().first.substr(0,10) << ","
              << "\"" + user_info.message_data.back().second + "\"" << ","
              << active_status << "\n";
    } else {
      outFile << "None," // Handle users with no messages
              << "None,"
              << "Inactive\n";
    }
  }

  //Close file
  outFile.close();
  std::cout << "Export to final_log.csv successfully! " << std::endl;

  return 0;
}
