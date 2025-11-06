<script lang="ts">
	import { rootApi } from "$lib/stores/globalValues";
    import { Input } from "$lib/components/ui/input/index.js";
    import { Button } from "$lib/components/ui/button/index.js";
	import { onMount } from "svelte";
	import { Toaster } from "svelte-sonner";

    let settings = {
        port: 0,
    };

    let changingSettings = $state({
        port: 0,
    });
    async function getSettings(){
        try {
            let result = await fetch(rootApi + "admin/api/getSettings");
            settings = await result.json();
           changingSettings = settings;
        } catch (error) {
            
        }
    }

    async function changeSettings() {
        try{
            let result = await fetch(rootApi + "admin/api/changeSettings", {
                method: "POST",
                headers: {
                    "Content-Type": "application/json",
                },
                body: JSON.stringify(changingSettings),
            })

            if (result.ok){
                if (changingSettings.port != settings.port) {
                    alert("Port changed, please change the port in url or in your reverse proxy config");
                }
            }
            await getSettings();
        }catch(error){
            alert("Failed to change settings");
        }
    }

    onMount(async () => {
        await getSettings();
    });
</script>

<div class="px-4 pb-4">
    <h1 class="mb-4">Settings</h1>
    <div class="my-2">
        <h3>Port</h3>
        <Input bind:value={changingSettings.port} type="number" placeholder="Port"></Input>
    </div>
    <div class="flex justify-end">
        <Button variant="secondary" 
        disabled={JSON.stringify(settings) == JSON.stringify(changingSettings)} 
        onclick={async ()=>{await changeSettings()}}>Save</Button>
    </div>
</div>