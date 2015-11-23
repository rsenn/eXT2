//	RandomFlamingLipsName.h - For generating random patch names etc.
//	--------------------------------------------------------------------------
//	Copyright (c) 2005 Niall Moody
//	
//	Permission is hereby granted, free of charge, to any person obtaining a
//	copy of this software and associated documentation files (the "Software"),
//	to deal in the Software without restriction, including without limitation
//	the rights to use, copy, modify, merge, publish, distribute, sublicense,
//	and/or sell copies of the Software, and to permit persons to whom the
//	Software is furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//	THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//	DEALINGS IN THE SOFTWARE.
//	--------------------------------------------------------------------------

#ifndef RANDOMFLAMINGLIPSNAME_H_
#define RANDOMFLAMINGLIPSNAME_H_

///	Simple class to generate a random Flaming Lips song name.
class RandomFlamingLipsName
{
  public:
	///	Constructor.
	RandomFlamingLipsName();

	///	Returns a random Flaming Lips song.
	char *GetName();
  private:
	char retval[24];
};

///	The array of all song titles.
const char FLSongNames[][24] = {
	"Bag Full of Thoughts",  //
	"Out For a Walk",
	"Scratchin' the Door",
	"Garden of Eyes",
	"Forever is a Long Time",
	"My Own Planet",
	"With You",
	"Unplugged",
	"Trains, Brains & Rain",
	"Just Like Before",
	"She is Death",
	"Man From Pakistan",
	"Godzilla Flick",
	"Staring at Sound",
	"Everything's Explodin'",
	"One Million Billionth..",
	"Maximum Dream for...",
	"Can't Exist",
	"Ode to C.C. (part 1)",
	"The Ceiling is Bendin'",
	"Prescription: Love",
	"Thanks to You",
	"Can't Stop the Spring",
	"Ode to C.C. (part 2)",
	"Love Yer Brain",
	"Right Now",
	"Michael, Time to Wake..",
	"Chrome Plated...",
	"Hari-Krishna Stomp...",
	"Miracle on 42nd St.",
	"Fryin' Up",
	"Hell's Angel Cracker...",
	"U.F.O. Story",
	"Redneck School of...",
	"Shaved Gorilla",
	"The Spontaneous...",
	"Last Drop of Morning...",
	"Begs and Achin'",
	"Shine on Sweet Jesus",
	"Unconsciously Screamin'",
	"Rainin' Babies",
	"Take Meta Mars",
	"Five Stop Mother...",
	"Stand in Line",
	"God Walks Among Us Now",
	"There You Are",
	"Mountain Side",
	"Lucifer Rising",
	"Ma, I Didn't Notice",
	"Let Me Be It",
	"God's a Wheeler Dealer",
	"Agonizing",
	"One Shot",
	"Cold Day",
	"Jam",
	"Golden Hearse",
	"Talkin' Bout the...",
	"...Smiling Deathporn...",
	"...Immortality Blues",
	"Hit Me Like You Did...",
	"The Sun",
	"Felt Good to Burn",
	"Gingerale Afternoon",
	"Halloween on the...",
	"The Magician vs. the...",
	"You Have to be Joking",
	"Frogs",
	"Hold Your Head",
	"Turn it On",
	"Pilot Can at the...",
	"Oh My Pregnant Head",
	"She Don't Use Jelly",
	"Chewin the Apple of...",
	"Superhumans",
	"Be My Head",
	"Moth in the Incubator",
	"*******",
	"When Yer Twenty Two",
	"Slow Nerve Action",
	"The Abandoned Hospital",
	"Psychiatric Exploration",
	"Placebo Headwound",
	"This Here Giraffe",
	"Brainville",
	"Guy who got a Headache",
	"When You Smile",
	"Kim's Watermelon Gun",
	"They Punctured My Yolk",
	"Lightning Strikes the..",
	"Christmas at the Zoo",
	"Evil Will Prevail",
	"Bad Days",
	"Okay I'll Admit That...",
	"Riding to Work in the..",
	"Thirty-Five Thousand...",
	"A Machine in India",
	"The Train Runs Over the",
	"How Will We Know?",
	"March of the Rotten...",
	"The Big Ol' Bug is the",
	"Race for the Prize",
	"A Spoonful Weighs a Ton",
	"The Spark That Bled",
	"Slow Motion",
	"What is the Light?",
	"The Observer",
	"Waitin' for a Superman",
	"Suddenly Everything has",
	"The Gash",
	"Feeling Yourself...",
	"Sleeping on the Roof",
	"Buggin'",
	"Fight Test",
	"One More Robot/...",
	"Yoshimi Battles the...",
	"...Pink Robots",
	"In the Morning of the",
	"Ego Tripping at the...",
	"Are You a Hypnotist?",
	"It's Summertime",
	"Do You Realize?",
	"All We Have is Now",
	"Approaching Pavonis...",
	"Assassination of the...",
	"I'm a fly in a Sunbeam",
	"Sunship Balloons",
	"A Change at Christmas"
};

#endif
