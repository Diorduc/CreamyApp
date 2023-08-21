#include <iostream>
#include <iomanip>
#include <limits>
#include <map>
#include <string>
#include <fstream>
#include <regex>
#include <ctime>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>

using std::cin;
using std::cout;
using std::endl;
using std::setw;
using std::string;

// A map to store user information (username and password)
std::map<std::string, std::string> users;

// Function to check if a password is strong
bool isStrongPassword(const std::string &password)
{
    // Define a regular expression pattern for a strong password
    std::regex passwordPattern("^(?=.*[a-z])(?=.*[A-Z])(?=.*[0-9])(?=.*[^a-zA-Z0-9]).{8,}$");
    return std::regex_match(password, passwordPattern);
}

// Function to register a user's information to a file
void registerUserToFile(const std::string &username, const std::string &password)
{
    std::ofstream file("users.txt", std::ios::app);
    if (file.is_open())
    {
        // Write user information to the file
        file << username << " " << password << std::endl;
        std::cout << "User registered successfully!" << std::endl;
    }
    else
    {
        std::cout << "Failed to open file!" << std::endl;
    }
}

// Function to register a new user
void registerUser()
{
    // Prompt user for details
    string firstname, lastname, age, username, password;
    cout << "Enter your first name: ";
    cin >> firstname;
    cout << "Enter your last name: ";
    cin >> lastname;
    cout << "Enter your age: ";
    cin >> age;
    cout << "Enter your desired username: ";
    cin >> username;

    // Check if the username already exists
    if (users.find(username) != users.end())
    {
        cout << "User already exists!" << endl;
        return;
    }

    // Prompt user to enter a strong password
    do
    {
        cout << "Enter password: ";
        cin >> password;

        if (!isStrongPassword(password))
        {
            cout << "Password must have at least 8 characters, including uppercase, lowercase, digits, and special characters." << endl;
        }
    } while (!isStrongPassword(password));

    // Store user information in the map and file
    users[username] = password;
    registerUserToFile(username, password);
}

// Function to log in a user
bool loginUser(string &loggedInUsername)
{
    std::string username, password;
    cout << "Enter username: ";
    cin >> username;

    // Check if the username exists in the map
    auto user = users.find(username);
    if (user == users.end())
    {
        cout << "Error!\n Invalid Username" << endl;
        return false;
    }

    // Prompt user to enter the correct password
    do
    {
        cout << "Enter password: ";
        cin >> password;

        if (user->second != password)
        {
            cout << "Incorrect password!" << endl;
        }
    } while (user->second != password);

    // Login successful
    cout << "Login successful!" << endl;
    loggedInUsername = username;
    return true;
}

// Function to determine the season based on the month
std::string determineSeason(int month)
{
    // Determine the season based on the month
    if (month >= 3 && month <= 5)
    {
        return "spring";
    }
    else if (month >= 6 && month <= 8)
    {
        return "summer";
    }
    else if (month >= 9 && month <= 11)
    {
        return "autumn";
    }
    else
    {
        return "winter";
    }
}

// Function to display the current time and season
void showCurrentTime()
{
    // Get the current time
    std::time_t currentTime = std::time(nullptr);
    std::tm *localTime = std::localtime(&currentTime);

    // Extract date and time components
    int year = localTime->tm_year + 1900;
    int month = localTime->tm_mon + 1;
    int day = localTime->tm_mday;
    int hour = localTime->tm_hour;
    int minute = localTime->tm_min;
    int second = localTime->tm_sec;

    // Display current date and time
    std::cout << "Current date and time: " << year << "-" << month << "-" << day << " "
              << hour << ":" << minute << ":" << second << endl;

    // Determine and display the current season
    string season = determineSeason(month);
    cout << "Current Season: " << season << endl;
}

// Callback function to write response from cURL
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

// Function to get the current temperature from an API
double getCurrentTemperature()
{
    double temperature = 0.0;

    string apiKey = "eaa4ffa92c7e4aa492184056232108";
    string apiUrl = "https://api.weatherapi.com/v1/current.json?key=" + apiKey + "&q=Bucharest";

    CURL *curl;
    curl = curl_easy_init();
    if (curl)
    {
        // Set cURL options
        curl_easy_setopt(curl, CURLOPT_URL, apiUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        // Perform the cURL request and get response
        std::string response;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK)
        {
            try
            {
                // Parse JSON response to get temperature
                nlohmann::json jsonData = nlohmann::json::parse(response);

                if (jsonData.contains("current") && jsonData["current"].contains("temp_c"))
                {
                    temperature = jsonData["current"]["temp_c"];
                }
                else
                {
                    cout << "Temperature data not found in JSON response." << endl;
                }
            }
            catch (const std::exception &e)
            {
                cout << "JSON parsing error: " << e.what() << endl;
            }
        }
        else
        {
            cout << "Failed to get temperature data." << endl;
        }

        // Clean up cURL
        curl_easy_cleanup(curl);
    }
    else
    {
        cout << "Failed to initialize cURL." << endl;
    }

    return temperature;
}

// Function to display the timer on the console
void displayClock(int hours, int minutes, int seconds)
{
    // Display the timer in a formatted manner
    cout << "\033[F";

    cout << std::setfill(' ') << setw(55) << "         TIMER         \n";
    cout << std::setfill(' ') << setw(55) << " --------------------------\n";
    cout << std::setfill(' ') << setw(29);
    cout << "| " << std::setfill('0') << setw(2) << hours << " hrs | ";
    cout << std::setfill('0') << setw(2) << minutes << " min | ";
    cout << std::setfill('0') << setw(2) << seconds << " sec |" << endl;
    cout << std::setfill(' ') << setw(55) << " --------------------------\n";
}

// Function for the timer and notifications
void timerAndNotifications(bool &timerActive, std::mutex &mtx, std::condition_variable &cv)
{
    int hours = 0;
    int minutes = 0;
    int seconds = 0;

    while (true)
    {
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [&timerActive]
                    { return timerActive; });
            if (!timerActive)
            {
                break;
            }
        }

        // Update and display the timer
        displayClock(hours, minutes, seconds);
        seconds++;

        if (seconds == 60)
        {
            minutes++;

            if (minutes == 60)
            {
                hours++;
                minutes = 0;
            }

            seconds = 0;
        }

        // Wait for one second before updating the timer again
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Notify when the timer stops
    cv.notify_all();
}

int main()
{
    // Load user data from a file
    std::ifstream file("users.txt");
    if (file.is_open())
    {
        std::string username, password;
        while (file >> username >> password)
        {
            users[username] = password;
        }
    }

    // Welcome message
    cout << std::setw(39) << "Welcome to Creamy" << endl;
    cout << "\"Creamy\" becomes the user's personal guide to a healthy tan and responsible sun exposure," << endl;
    cout << "bringing information and assistance every step of the way." << endl;
    cout << endl;
    cout << " Press Enter to begin";
    cout << endl;
    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    // Thread and variables for timer functionality
    std::thread timerThread;
    bool timerActive = false;
    std::mutex mtx;
    std::condition_variable cv;

    do
    {
        int choice;
        cout << "1. Register\n2. Login\n3. Quit" << endl;
        cout << "Enter your choice: ";
        cin >> choice;
        string loggedInUsername;

        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (choice)
        {
        case 1:
            registerUser();
            break;
        case 2:
            if (loginUser(loggedInUsername))
            {
                // Display user information
                cout << "Welcome back " << loggedInUsername << "!" << endl;
                showCurrentTime();
                double currentTemperature = getCurrentTemperature();
                if (currentTemperature != 0.0)
                {
                    cout << "Current Temperature: " << currentTemperature << " °C" << endl;
                }
                else
                {
                    cout << "Failed to get temperature data.";
                }

                // Start timer if desired
                cout << "Do you want to start the sun exposure timer? (yes/no): ";
                string startTimerChoice;
                cin >> startTimerChoice;

                if (startTimerChoice == "yes")
                {
                    cout << "Sun exposure timer started." << endl;
                    timerActive = true;
                    timerThread = std::thread(timerAndNotifications, std::ref(timerActive), std::ref(mtx), std::ref(cv));
                }
                else
                {
                    cout << "Sun exposure timer not started." << endl;
                }

                // Stop the timer if it was active
                if (timerActive)
                {
                    timerThread.join();
                    timerActive = false;
                    cout << "Timer stopped." << endl;
                }
            }
            break;
        case 3:
            if (timerActive)
            {
                // Stop the timer before quitting
                timerActive = false;
                cv.notify_all();
                timerThread.join();
                cout << "Timer stopped." << endl;
            }
            return 0;
        default:
            break;
        }
    } while (true);

    return 0;
}
