<h3>About Free Sentinel GL</h3>
<p>
This remake of the 1986 C64 Firebird classic 'The Sentinel' was written during the weeks between April 29th 2015 and May 25th 2015 as a self-tutoring project in order to learn how to work with openGL in C++. It is also my very first GitHub online project which I hereby declare to be open source under the GPL 3 license! :-)
</p>
<p>
The object of the game is to ascend the mountain and replace the Sentinel on the top
of his tower. Only from there a hyperjump will allow the synthoid (i.e. the robot
consciousness that is the player) to progress into a new landscape.
</p>
<p>
3D models were done in Blender, most of the textures were photographed somewhere between Bochum and Frankfurt am Main in Germany using my trusty smartphone and put together using the Gimp. Sound samples I took using the App "Smart Voice Recorder" (writing directly into .wav files) and pepped them for the game using
Audacity. Development itself was done using both vim and KDevelop.
</p>
<h3>Installation procedure</h3>
<p>
So far this program was written with Linux in mind. However, being based on pure Qt 5.4 it should
be a quite straight-forward task to bring it to alternative operative systems.

<h4>Prerequisites</h4>
On a Linux environment you are going to need
<ol>
<li>A c++ compiler (I use g++ (Debian 4.9.2-10) 4.9.2; I read a comprehensive Debian/Ubuntu package containing all one needs would be <code>build-essential</code>)</li>
<li><code>cmake</code> support for compilation (version &ge; 2.8.11)</li>
<li>appropriate openGL support. GLSL shaders use '#version 120' i.e. they require openGL 2.1. The program is tested using openGL version 3.0 though. I am not entirely sure, but I believe on a Debian-like system you ought to be fine with installing <code>mesa-common-dev</code></li>
<li>Qt&ge;5.4. If your distribution does not yet support it directly download and install it from <a href="http://www.qt.io/download/">http://www.qt.io/download/</a></li>
<li>You may also want <code>git</code> for a stylish download of <i>Free Sentinel GL</i>.</li>
</ol>
</p>
<p>
<h4>Installation procedure</h4>
<ol>
<li>Download this repository to your computer. To this end, using git, go to the directory you would like to have the program in and say <pre>git clone https://github.com/kochsoft/free_sentinel_gl
</pre></li>
<li>Within <code>./free_sentinel_gl/sentinel/build</code> say <code>cmake .</code> (mind the period symbol! It is important) to parse the CMakeLists.txt file into the required make file set. If cmake fails it probably could not find your Qt 5.4 installation. In that case take a look at the trouble shooting section below.</li>
<li>Still within <code>./free_sentinel_gl/sentinel/build</code> say <code>make</code></li>
<li>Then <b>be patient</b>. Especially the building process of <code>qrc_application.cpp.o</code> may take some time. There are quite a few resource files (most notably the texture graphics) to be processed.
On my system (an acer aspire 5750g laptop with an 8 core intel i7-2630QM) it takes about an hour.</li>
</ol>
</p>
<p>
This should compile the game on your PC generating within <code>./free_sentinel_gl/sentinel/build</code> the executable <code>./sentinel</code>
</p>
<h3>Trouble-shooting</h3>
<h4>'cmake .' does not seem to find my Qt 5.4 installation.</h4>
<p>
If you have, like me on my almost clean Debian machine, a local installation of Qt 5.4 it may
become necessary to set the cmake prefix path within <code>./free_sentinel_gl/sentinel/build/CMakeLists.txt</code> to something appropriate
so that CMake may find its Qt 5.4 cmake files.
</p>
<p>
On my machine this would be
</p>
<pre>
set(CMAKE_PREFIX_PATH "/home/kochmn/sw/Qt/Qt_5_4/5.4/gcc_64/lib/cmake/")
</pre>
<p>
You will find such a line commented out within <code>CMakeLists.txt</code>. Just decomment it and adjust the path to match the situation on your own system. Then try calling <code>cmake .</code> (still minding the period symbol) again. Note: After a failed first attempt CMake may already have generated some files from a putative system Qt of lower version contradicting the correct local Qt installation. This may lead to errors like <i>QOpenGLWidget not found</i>. If that happens simply run <code>./clean.sh</code> and try <code>cmake . && make</code> again.
</p>

<h4>I'd like to change the framerate.</h4>
Some options, most notably the framerate setting (which defaults to 24), are not yet set into
a proper Qt setup window. These settings may be found and adjusted within
<pre>
./free_sentinel_gl/sentinel/src/include/config.h
</pre>

<h4>I don't like the default settings for some of the game options.</h4>
For this, too, there is no setup form. Nor are these options held within config.h.
Instead you will have to tinker with the qt user interface XML file,
<code>/free_sentinel_gl/sentinel/src/ui/dialog_setup_game.ui</code>, directly.
This is easiest using the Qt designer.

<h3>Acknowledgements</h3>
<ul>
<li>The eye icon of the game is based on an Eye of Horus icon I found on <a href="http://www.clipartbest.com/">http://www.clipartbest.com/</a>. I would like to credit the artist here, however no name was attached to the graphic.
</li>
<li>The dome image of the mars scenery is based on a picture taken by the Pathfinder in 1999 as depicted in Wikipedia: <a href="http://en.wikipedia.org/wiki/Extraterrestrial_skies">http://en.wikipedia.org/wiki/Extraterrestrial_skies</a>. The original of the icon hidden on that scene may be found <a href="http://en.wikipedia.org/wiki/Curiosity_%28rover%29">here</a>.
</li>
<li>The sky above the moon scenery, too, is based on a picture I found on Wikipedia:
<a href="http://en.wikipedia.org/wiki/Extraterrestrial_skies">http://en.wikipedia.org/wiki/Extraterrestrial_skies</a>. The picture originally was taken during the Apollo VIII mission on December 24th 1968.
Also I credit A.C. Clarke for the inspiration to an especially sentinel-like 'moon tree'
bearing the most simplistic texture ever :-)</li>
<li>For the asteroidical scenery Wikipedia was still absolutely indispensible.
First the horizon is a section
of P67 <a href="http://en.wikipedia.org/wiki/67P/Churyumov%E2%80%93Gerasimenko">http://en.wikipedia.org/wiki/67P/Churyumov%E2%80%93Gerasimenko</a> as photographed by Rosetta in September 2014. Second the hidden icon in the background is the artistic picture on <a href="http://en.wikipedia.org/wiki/Rosetta_%28spacecraft%29">http://en.wikipedia.org/wiki/Rosetta_%28spacecraft%29</a> or more precisely
on <a href="https://www.flickr.com/photos/europeanspaceagency/11206647984/">https://www.flickr.com/photos/europeanspaceagency/11206647984/</a>. There they state anonymously: 'Credit: ESA/ATG medialab'.
The stellar background is a depiction of the horsehead nebula: <a href="http://en.wikipedia.org/wiki/Horsehead_Nebula">http://en.wikipedia.org/wiki/Horsehead_Nebula</a>.
</li>
<li>My gratitude also goes to numerous people who seemingly enjoy answering noob questions on the <a href="stackoverflow.com">stackoverflow.com</a> forum. Their competent advice was invaluable for my work on this program.</li>
</ul>

<h3>Prominent ToDos</h3>
<ul>
<li>Wait for Qt 5.4 to come out of 'experimental' and build a debian package.</li>
<li>Do a windows port in .NET.</li>
</ul>
