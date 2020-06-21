# RLCSVPlugin

RLCSVPlugin is a BakkesMod plugin that saves Rocket League scoreboard stats for all players to CSV files on the conclusion of an online match.

## Installation
* Install the plugin through [bakkesplugins.com](https://bakkesplugins.com/plugins/view/94)

## Settings
* With Rocket League open press F2 -> Plugins -> RLCSVPlugin
* Save directory: The location where you would like your CSV files to be saved
  * Default is `bakkesmod/data/RLCSV/`
* Save directory structure: Saves CSV files into folders based on date and game mode. Deselecting this setting will save all CSV's regardless of date and game mode in the same folder
  * Example: `bakkesmod/data/RLCSV/2020-29-03/Standard/`

## Updates
**2020-03-29**:
* Add setting change save directory
* Add setting to change save directory structure
* Add timestamp, game mode, and match result for each player in CSV file
