function post_choice(choice) {
    const logo = document.querySelector("#" + choice);
    logo.classList.add("connecting");
    logo.setAttribute("disabled", true);
    var xhr = new XMLHttpRequest();
	const method = "POST";
	const url =  "/";
	xhr.open(method, url, true);
    //Send the proper header information along with the request
    xhr.setRequestHeader("Content-Type", "text/plain");

	// Call a function when the state changes
    xhr.onreadystatechange = async function() { 
    	if (this.readyState === XMLHttpRequest.DONE) {
			const status = xhr.status;
			if (status === 0 || (status >= 200 && status < 400)) {
	 	   		// Request finished. Do processing here.
		    	console.log("Success! Redirecting...");
				console.log(xhr); // DEBUGGING
		    	logo.classList.remove("connecting");
		    	logo.classList.add("connected");
		    	document.getElementById("status").innerHTML = "redirecting, please wait...";
			    await new Promise(r => {
		        setTimeout(r, 2000)
		    	});
		    if(choice === "alexandria") {
		        choice = "kavita"
		    }
		    window.open("https://" + choice + ".oncoto.app");
    		} else {
	    		logo.classList.remove("connecting");
	    		logo.classList.remove("connected");
	   			logo.setAttribute("disabled", false);
    		}
    	}
	};
    xhr.send(choice);
    document.getElementById("status").innerHTML = "loading service...";
}
