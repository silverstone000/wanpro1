# Link-state routing simulation
Simulation of Link-State Routing Over Application Layer

<h2>Prerequisite</h2>
<p>Devoloped under</p> 
<p>CentOS 7 1810.</p>
<p>C++ Boost library 1.69.0</p>
<p>g++ 4.8.5</p>

<h2>Compile using visualstudio 2017.</h2>

<ol>
<li>Connect to remote linux system in crossplatform options(see included readme of VS2017)</li>
<li>In project property, add additional command "-pthread"</li>
<li>Build</li>
</ol>

<h2>Commandline used by VS as reference</h2>
<p>
<code>
"g++" -W"switch" -W"no-deprecated-declarations" -W"empty-body" -W"conversion" -W"return-type" -W"parentheses" -W"no-pointer-sign" -W"no-format" -W"uninitialized" -W"unreachable-code" -W"unused-function" -W"unused-value" -W"unused-variable" -std=c++11 -Wall -fno-strict-aliasing -g2 -gdwarf-2 "g++" -O0 "3600000" -fthreadsafe-statics -W"switch" -W"no-deprecated-declarations" -W"empty-body" -W"conversion" -W"return-type" -W"parentheses" -W"no-format" -W"uninitialized" -W"unreachable-code" -W"unused-function" -W"unused-value" -W"unused-variable" -frtti -fno-omit-frame-pointer -std=c11 -fexceptions "1" -o "directory\to\obj\output\%(filename).o" 
</code>
<p>and</p>
<code>
-o"directory\to\bin\output\%(filename).owanpro1.out" "3600000" -Wl,-z,relro -Wl,-z,noexecstack -Wl,--no-undefined "g++" -Wl,-z,now 
</code>
</p>
