#include "PCH.h"
class Guffer {
public:
	void SayGuff(float deltaTime) {
		guffTimer += deltaTime;

		if (guffTimer > SAY_GUFF_EVERY_X_SECONDS) {
			guffTimer = 0;
			int index = rand() % guff.size();
			std::cout << guff[index] << "\n";
		}
	}
private:
	float guffTimer = 0;
	const float SAY_GUFF_EVERY_X_SECONDS = 10.0f;

	std::vector<std::string> guff = {
		"Combing Tree Leaves",
		"Initializing X-Mainframe",
		"Decoupling inter point entanglement",
		"Converting points to 4D space",
		"Updating tree 837,149",
		"Returning triangles to quantum state",
		"Upsetting line based rendering",
		"Placing points meticulously",
		"Cancelling metric capture... Just kidding",
		"Today's weather is 5C with clear skies",
		"Today's weather is not 5C with clear skies",
		"Returning Space Cowboy",
		"Searching for point 145,101,341...",
		"Converting points -> more points",
		"Confusing mesh shader and meshes",
		"Enjoying views of Changhaizi lake",
		"Decombobulating point matrix",
		"Working alongside the thingamajig",
		"Debugging the dohickey",
		"Analysing the whatchamicallit",
		"Counting points... More than 1 detected",
		"Undoing last item...",
		"Thinking of next thing to say...",
		"This is the rare status message, be proud!",
		"Hiding a squirrel within the scene...",
		"Adding sphere of certaity (9th Dimention Sequence)",
		"We apologise for any incorrectly spelt messages.",
		"Today is the 9th of May 2023",
		"Treasure hunt! 42.6411, 18.1106",
		"Working hard and hardly working",
		"Removing a tree from the scene...",
		"Wishing there was more pins in a bowling alley",
		"Wishing there was less pins in a bowling alley",
		"Being thankful for 10 being the perfect number of pins in a bowling alley",
		"GPU overheating. Enabling extinguishing procedure",
		"Comparing trees inside to the trees outside.",
		"Making trees taller",
		"Heating doughnuts via laptop fan",
		"Disabling architectural changes to interior dependencies...",
		"Fixing bugs...",
		"Measuring volume of points per unit area",
		"Calculating optimal point shape for maximum temprature reflection",
		"Calculating interior humidity of a tree",
		"Message Under Construction",
		"0.1% complete [X---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------]",
	};
};