import { rootApi } from "$lib/stores/globalValues";

async function authenticate(){
    try {
        let result = await fetch(rootApi+"api/authenticate", {
            method: "GET",
        });

        if (result.ok) 
            return true;
        else 
            return false;
    } catch (error) {
        return false
    }
}

async function authorize(){
    try {
        let result = await fetch(rootApi+"api/authorize", {
            method: "GET",
        });

        if (result.ok) 
            return true;
        else 
            return false;
    } catch (error) {
        return false
    }
}

async function getInfo(){
    try {
        let result = await fetch(rootApi+"api/info", {
            method: "GET",
        });

        return result.text();
    } catch (error) {
        return " ";
    }
}

export {authenticate, authorize, getInfo}