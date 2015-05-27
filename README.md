<p>
# free_sentinel_gl
C++, Qt 5.4, OpenGL 2.0 remake of the Firebird C64 classic 'The Sentinel'. For Linux, using CMake.
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
Audacity. Development itself was done using both vim and KDevelop. The eye icon of the game is
based on an Eye of Horus icon I found on http://www.clipartbest.com/. I would like to credit the
artist here, however no name was attached to the graphic.
</p>
<h3>Installation procedure</h3>
<p>
So far this program was written with Linux in mind. However, being based on pure Qt 5.4 it should
be a quite straight-forward task to bring it to alternative operative systems.

<h4>Prerequisits</h4>
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
