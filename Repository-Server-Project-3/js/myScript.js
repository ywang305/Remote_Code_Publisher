/*////////////////////////////////////////////////////////////////////////////
// myScript.js -  expand, collpase class body, method, global funcitons    //
// ver 1.0                                                                 //
// ----------------------------------------------------------------------- //
// copyright  Yaodong Wang, 2017                                           //
// All rights granted provided that this notice is retained                //
// ----------------------------------------------------------------------- //
// Language:    Visual C++, Visual Studio 2015                             //
// Platform:    HP-Q5F5, Core i3, Windows 10                               //
// Application: Project #3, CSE687 - Object Oriented Design, S2015         //
// Author:      Yaodong Wang , Syracuse University                         //
//              ywang305@syr.edu                                           //
///////////////////////////////////////////////////////////////////////////*/


// -- < onload to set packages as collpased other than current pakage as expanded >--

window.onload = function () {
    var title = document.getElementsByTagName("header")[0].innerHTML;
    var len = document.getElementsByClassName("c0").length;
    for( var i=0; i<len; i++)
    {
        if (title == document.getElementsByClassName("c0")[i].getAttribute("id")) {
            document.getElementsByClassName("c0")[i].style.display = 'block';
        } else {
            document.getElementsByClassName("c0")[i].style.display = 'none';
        }
    }
}

// -- < toggle, to resposne onlick to collpase or expand list >--

function Toggle(ID) {
    var x = document.getElementById(ID);
    if (x.style.display == 'none') {
        x.style.display = 'block';
    } else {
        x.style.display = 'none';
    }
}