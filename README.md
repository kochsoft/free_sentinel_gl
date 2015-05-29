<p>
# free_sentinel_gl
C++, Qt 5.4, OpenGL 2.1 remake of the Firebird C64 classic 'The Sentinel'. For Linux, using CMake.
</p>
<h3>About</h3>
<p>
There already is a Wiki page: <a href="https://github.com/kochsoft/free_sentinel_gl/wiki/Free-Sentinel-GL">https://github.com/kochsoft/free_sentinel_gl/wiki/Free-Sentinel-GL</a>
</p>
<p>
This remake was written by Markus-Hermann Koch
during the weeks between April 29th 2015 and May 25th 2015 as a self-tutoring project in order
to learn how to work with openGL in C++. 3D models were done in Blender, textures where photographed
using my trusty smartphone and put together using the Gimp. Sound samples I took using the App
"Smart Voice Recorder" (writing directly into .wav files) and pepped them for the game using
Audacity. Development itself was done using both vim and KDevelop.
</p>
<h3>Installation procedure</h3>
<p>
So far this program was written with Linux in mind. However, being based on pure Qt 5.4 it should
be a quite straight-forward task to bring it to alternative operative systems.

<h4>Prerequisites</h4>
On a Linux environment you are going to need
<ol>
<li>cmake support for compilation</li>
<li>Qt >=5.4</li>
<li>appropriate openGL support (TODO: Specify! At any rate the shaders are written
in 'modern openGL', version 3 on my computer. However, I believe 2.0 should be fine).</li>
</ol>
</p>
<p>
<h4>Installation procedure</h4>
<ol>
<li>Download this repository to your computer.</li>
<li>Within ./build say <code>cmake .</code> (mind the period symbol! It is important) to parse CMakeLists.txt into everything needed in order to</li>
<li>still within ./build say <code>make</code></li>
</ol>
</p>
<p>
This should compile <i>Free Sentinel GL</i> on your PC generating within ./build the executable <code>./sentinel</code>
</p>
<h3>Trouble-shooting</h3>
<p>
If you have, like me on my almost clean Debian machine, a local installation of Qt 5.4 it may
become necessary to set your cmake prefix path within the CMakeLists.txt to something appropriate
so that CMake may find its Qt 5.4 cmake files.
</p>
<p>
On my machine this would be
</p>
<pre>
set(CMAKE_PREFIX_PATH "/home/kochmn/sw/Qt/Qt_5_4/5.4/gcc_64/lib/cmake/")
</pre>
<p>
You will find such a line commented out within CMakeLists.txt. Just decomment it and adjust it
as befits your own system.
</p>
<p>
<ul>
<li>The eye icon of the game is based on an Eye of Horus icon I found on <a href="http://www.clipartbest.com/">http://www.clipartbest.com/</a>. I would like to credit the artist here, however no name was attached to the graphic.
</li>
<li>The dome image of the mars scenery is based on a picture taken by the Pathfinder in 1999 as depicted in Wikipedia: <a href="http://en.wikipedia.org/wiki/Extraterrestrial_skies">http://en.wikipedia.org/wiki/Extraterrestrial_skies</a>. The original of the icon hidden on that scene may be found <a href="http://en.wikipedia.org/wiki/Curiosity_%28rover%29">here</a>.
</li>
<li>The sky above the moon scenery, too, is based on a picture I found on Wikipedia:
<a href="http://en.wikipedia.org/wiki/Extraterrestrial_skies">http://en.wikipedia.org/wiki/Extraterrestrial_skies</a>. The pciture originally was taken by Apollo 8 on December 24th 1968.
Also I credit A.C. Clarke for the inspiration to a 'moon tree'
bearing the most simplistic texture ever :-)</li>
<li>For the asteroidical scenery Wikipedia was still absolutely indispensible.
First the horizon is a section
of P67 <a href="http://en.wikipedia.org/wiki/67P/Churyumov%E2%80%93Gerasimenko">http://en.wikipedia.org/wiki/67P/Churyumov%E2%80%93Gerasimenko</a> as photographed by Rosetta in September 2014. Second the hidden icon in the background is the artistic picture on <a href="http://en.wikipedia.org/wiki/Rosetta_%28spacecraft%29">http://en.wikipedia.org/wiki/Rosetta_%28spacecraft%29</a> or more precisely
on <a href="https://www.flickr.com/photos/europeanspaceagency/11206647984/">https://www.flickr.com/photos/europeanspaceagency/11206647984/</a>. There they state anonymously: 'Credit: ESA/ATG medialab'.
The stellar background is a depiction of the horsehead nebula: <a href="http://en.wikipedia.org/wiki/Horsehead_Nebula">http://en.wikipedia.org/wiki/Horsehead_Nebula</a>.
</li>
<li>My general gratitude goes to the people who seemingly enjoy answering noob questions on the <a href="stackoverflow.com">stackoverflow.com</a> forum. There were some times I would have been seriously stuck without their help.</li>
</ul>
</p>

