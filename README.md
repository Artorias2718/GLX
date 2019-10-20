# GLX

Since I struggled so much to get through this in school, I thought I'd share what I learned from the experience.

### Prerequisites

* PC running a Windows 7, 8, 8.1, or 10 OS.
* [DirectX 11 SDK](https://www.microsoft.com/en-us/download/confirmation.aspx?id=6812)
  * In case you're having trouble installing it (S1023 error), check out this [page](https://support.microsoft.com/en-us/help/2728613/s1023-error-when-you-install-the-directx-sdk-june-2010) for a solution.
* [Microsoft Visual Studio 2017 Community](https://visualstudio.microsoft.com/downloads/)
* Enable Graphics Tools Feature
	* Go to Settings
	* Search for "Optional"
	* Click on "Manage optional features"
	* Click on "Add a feature"
	* Select "Graphics tools"


## Getting Started

Once you have done all of the DirectX setup (I know it's tedious but this version of DirectX is technically an EOL product), open up the solution file. Make sure the CTF project is the startup project (the project's name should be in bold text under "Solution Explorer". If not, CTF -> Right-click -> Set As Startup Project)

* CTF -> Right-click -> Properties -> Debugging
* Make sure to select "All Configurations" and "All Platforms"
* Type in $(GameDir) for the "Working Directory" value
* If everything's setup, you should see a small white triangle in the center of a 512 x 512 window. This should look the same on both Debug/Release configurations of x86 (OpenGL) and x64 (DirectX)


## Authors

* **[Arturo Ordonez-Hernandez](https://artorias2718.wordpress.com/)**


## Acknowledgments

* [EAE](https://eae.utah.edu), University of Utah