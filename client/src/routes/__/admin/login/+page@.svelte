<script lang="ts">
	import { goto } from "$app/navigation";
	import { authenticate, authorize } from "$lib/auth/info";
	import Button from "$lib/components/ui/button/button.svelte";
	import { Input } from "$lib/components/ui/input";
    import {logo} from "$lib/images/base64"
	import { rootApi } from "$lib/stores/globalValues";
	import { Loader2Icon } from "lucide-svelte";
	import { onMount } from "svelte";
	import { toast, Toaster } from "svelte-sonner";

    let loginForm = $state({
        username: "",
        password: ""
    });
    

    let isLogining = $state(false)
    async function login(){
        isLogining = true;
        try{
            let result = await fetch(rootApi + "api/login",{
                method: "POST",
                headers:{
                    "Content-type": "application/json"
                },
                body: JSON.stringify(loginForm)
            })

            if (result.ok){
                toast.success("Logined", {
                    duration: 1000
                })
                await 
                goto("/__/admin")
            }else{
                toast.error("Invalid username or password", {
                    duration: 5000
                })
                isLogining = false;
            }
        }catch{

        }
    }

    onMount(async () => {
		let isLocalhost: boolean;

		if (typeof window !== 'undefined') isLocalhost = window.location.hostname === 'localhost';
		else isLocalhost = false;

		if (isLocalhost) {
			goto('/__/admin');
		}

        if ((await authenticate()) && (await authorize())) {
            goto('/__/admin');
        }
	});
</script>

<svelte:head>
	<title>LiliumFS Admin Login</title>
</svelte:head>
	
<Toaster
	position="top-center"
	toastOptions={{
		unstyled: true,
		classes: {
			toast: 'bg-popover border rounded-sm px-4 py-2 flex items-center gap-4',
			description: 'text-primary text-sm',
			icon: 'text-lg'
		}
	}}
/>

<div class="h-screen flex justify-center items-center drop-shadow-sm drop-shadow-accent">
    <div class="flex flex-col w-2xl bg-accent p-4 rounded-sm gap-2">
        <img class="w-70 mb-2" src={logo} alt="">
        <div class="flex flex-col gap-4">
            <Input bind:value={loginForm.username} placeholder="Username"/>
            <Input bind:value={loginForm.password} type="password" placeholder="Password"/>
        </div>
        {#if isLogining}
            <div class="flex justify-end">
                <Button variant="secondary" disabled><Loader2Icon class="animate-spin"/> Login</Button>
            </div>
        {:else}
            <div class="flex justify-end">
                <Button variant="secondary" onclick={async ()=>{await login()}}>Login</Button>
            </div>
        {/if}
    </div>
</div>