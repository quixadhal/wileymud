function toggleDiv(divID) {
    element = document.getElementById(divID);
    if(element !== undefined && element !== null) {
        if(element.style.display == 'none') {
            element.style.display = 'block';
        } else {
            element.style.display = 'none';
        }
    }
}
function showDiv(divID) {
    element = document.getElementById(divID);
    if(element !== undefined && element !== null) {
        element.style.display = 'block';
    }
}
function hideDiv(divID) {
    element = document.getElementById(divID);
    if(element !== undefined && element !== null) {
        element.style.display = 'none';
    }
}
function scroll_to(id) {
    document.getElementById(id).scrollIntoView({behavior: 'smooth', block: "center"});
}
function dim(element) {
    if(element !== null) {
        element.style.opacity = "0.5";
        element.classList.remove('glowing');
        //element.className = "not-glowing";
    }
}
function brighten(element) {
    if(element !== null) {
        element.style.opacity = "1.0";
        element.classList.add('glowing');
        //element.className = "glowing";
    }
}
function at_bottom() {
    var body = document.body;
    var html = document.documentElement;
    var doc_height = Math.max( body.scrollHeight, body.offsetHeight,
        html.clientHeight, html.scrollHeight, html.offsetHeight );
    var atBottom = false;
    if( window.innerHeight >= doc_height ) {
        // The page cannot scroll... so yes, but no.
    } else if( (window.innerHeight + window.pageYOffset) >=
        (document.body.offsetHeight) ) {
        // We are at the bottom of the page.
        atBottom = true;
    }
    return atBottom;
}
