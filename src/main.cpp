#include <cpr/response.h>
#include <iostream>
#include <fstream>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <string>


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
  const std::string CHANNEL_ID = "1307090118577750147";
  const std::string URL = "https://discord.com/api/v10/channels/" + CHANNEL_ID + "/messages?limit=10";

  //Make GET request
  cpr::Response reponse = cpr::Get(
      cpr::Url{URL},
      cpr::Header{
        {"Authorization", "Bot " + BOT_TOKEN}
      }
  );

  //Check return Status and parse JSON 
  if (reponse.status_code == 200) {
    nlohmann::json json_response = nlohmann::json::parse(reponse.text);

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
  }

  return 0;

}
