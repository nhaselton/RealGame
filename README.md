I wanted to try to make as much of a 3D game from scratch as I could. Over the years i've learned many parts of game engine / game development but never put it together into something.
I just Graduated and decided I wanted to spend a few months relaxing and working on this. The game isn't very fun but it does work as intended. 

To compile: Run realgame.sln using VS 2020

Souce Tree Breakdown
  CUM: Map Compilier. Takes a trenchbroom .map file and converts it into useable data for the game
  Light: Lightmapper, takes a game ready map file and simulates per texel lighting for the entire enviroment
  Ext: External libraries such as audio, Window creation, json parsing and texture loading.
  RealGame: Game Source code.
        Res: model files, map files, etc.
