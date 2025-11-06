<script lang="ts">
    import {onMount} from 'svelte';
    import {rootApi} from '$lib/stores/globalValues';
    import { HardDrive, Folder, File, CircleChevronLeft, Loader2Icon, House } from 'lucide-svelte';
    import { Input } from "$lib/components/ui/input/index.js";
    import { Button } from "$lib/components/ui/button/index.js";
    
    let { afterAdd, isVirtual = false, virtualRootName = ""} = $props();
    
    let drives: string[] = $state([]);
    let currentDrive: string = $state("");
    let path: string[] = $state([]);
    let dirContents: any = $state([]);
    let selected = $state({
        folders: [],
        files: []
    });
    let selectedCount = $derived(selected.folders.length + selected.files.length);
    let input: string = $derived(currentDrive + "/" + path.join("/"));
    let fetchSuccess: boolean = $state(true);
    let isFetching: boolean = $state(false);

    async function getDrives(): Promise<void>{
        isFetching = true;
        emptySelected();
        currentDrive = "";
        path = [];
        const result = await fetch(rootApi+"admin/api/getDrives");
        drives = await result.json();
        isFetching = false;
        fetchSuccess = true;
    }

    async function getDirContents() {
        isFetching = true;
        emptySelected();
        const _path = currentDrive + "/" +path.join("/");
        
        try{
            const result = await fetch(rootApi+"admin/api/getDirectoryContents/" + _path)
            dirContents = await result.json();
            fetchSuccess = true;
        }catch(e){
            
            fetchSuccess = false;
        }
        
        isFetching = false;
    }

    
    async function getDriveContents(_name: string) {
        currentDrive = _name;
        await getDirContents();
    }
    
    function addPath(_name: string) {
        path.push(_name);
    }

    async function getInputPath(){
        if (input == "") {
            await getDrives()
        }else{
            while (input.includes("\\") || input.includes("//")) {
                input = input.replaceAll("\\", "/");
                input = input.replaceAll("//", "/");
            }
            const args = input.split("/");
            currentDrive = args[0];
            args.splice(0, 1);
            if (args[args.length-1] == "") args.pop();
            path = args;
            await getDirContents();
        }
    }

    async function goBack() {
        if (path.length == 0) {
            await getDrives();
        }
        if (path.length > 0) {
            path.pop();
            await getDirContents();
        }
    }

    function emptySelected() {
        selected = {
            folders: [],
            files: []
        }
    }

    async function add(){
        try {
            let req: {folders: any[]; files: any[]; name: string}  = {
                folders: [],
                files: [],
                name: "",
            }
            for (let folder of selected.folders){
                const item: any = {
                    src: (currentDrive + (currentDrive != "" ? "/"+path.join("/") + "/" :  "")).replaceAll("//", "/") + folder,
                    name: folder
                }
                req.folders.push(item);
            }

            for (let file of selected.files){
                const item: any = {
                    src: (currentDrive + (currentDrive != "" ? "/"+path.join("/") + "/" :  "")).replaceAll("//", "/") + file,
                    name: file
                }
                req.files.push(item);
            }
            
            let endPoint = "admin/api/addSources"
            
            
            if (isVirtual){
                req.name = virtualRootName;
                endPoint = "admin/api/addVirtualChild"
            }
            
            let result = await fetch(rootApi+endPoint, {
                method: "POST",
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify(req)
            });
           
        }catch(e){
            
        }
        
    }
    
    onMount(async () => {
        await getDrives();
    })

    $inspect(currentDrive,path)

</script>


<div class="flex flex-col p-4 w-full h-full bg-background overflow-x-hidden">
    <div class="flex w-full items-center mb-4">
        <Button variant="secondary" size="icon" class="mr-2 cursor-pointer" onclick={goBack} >
            <CircleChevronLeft />
        </Button>
        <Button variant="secondary" size="icon" class="mr-2 cursor-pointer" onclick={async ()=>{await getDrives()}}>
            <House />
        </Button>
        <Input type="text" placeholder="Path" 
        onkeypress={
            (e)=>{
                if (e.key == "Enter") getInputPath();
            }
        } 
        bind:value={input} class="max-w-xs"></Input>
    </div>
    <div class="flex-1 mb-4 overflow-y-auto">
        <div class="flex flex-col w-full">
            {#if isFetching}
                <div class="flex justify-center items-center h-full">
                    <Loader2Icon class="animate-spin"/>
                </div>
            {:else}
                {#if !fetchSuccess}
                    <div class="text-red-500 text-center mt-4">Error fetching directory contents</div>
                {:else}
    
                    {#if currentDrive == ""}    
                        {#each drives as drive}
                        <div class="flex items-center mb-2 p-2 cursor-pointer rounded-sm hover:bg-accent">
                            <input type="checkbox" name="path" value={drive} bind:group={selected.folders}>
                            <div role="none" class="flex flex-1 items-center ml-4 break-words" onclick={async ()=>{await getDriveContents(drive)}}>
                                <HardDrive/>
                                <span class="ml-2 flex-1">{drive}</span>
                            </div>
                        </div>
                        {/each}
                    {:else}
                        {#each dirContents.folders as item}
                        <div class="flex items-center mb-2 p-2 cursor-pointer rounded-sm hover:bg-accent">
                            <input type="checkbox" name="path" value={item} bind:group={selected.folders}>
                            <div role="none" class="flex flex-1 items-center ml-4 break-words" onclick={async ()=>{addPath(item); await getDirContents()}}>
                                <Folder/>
                                <span class="ml-2 flex-1">{item}</span>
                            </div>
                        </div>
                        {/each}
                
                        {#each dirContents.files as item}
                        <div class="flex items-center mb-2 p-2 cursor-pointer rounded-sm hover:bg-accent">
                            <input type="checkbox" name="path" value={item} bind:group={selected.files}>
                            <div role="none" class="flex flex-1 items-center ml-4 break-words">
                                <File/>
                                <span class="ml-2 flex-1">{item}</span>
                            </div>
                        </div>
                        {/each}
                        
                    {/if}
                {/if}
            {/if}
        </div>
    </div>
    <div class="flex items-center justify-end">
        <Button onclick={async ()=>{await add(), await afterAdd()}} class="cursor-pointer" variant="secondary" disabled={selectedCount == 0 ? true : false} >Add{selectedCount > 0 ? ` ${selectedCount}` : ''}</Button>
    </div>
</div>

<style>
    .break-words{
        word-break: break-word;
    }
</style>


