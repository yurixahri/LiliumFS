<script lang="ts">
	import { EVENTS, rootApi, rootUrl } from "$lib/stores/globalValues";
	import { onMount } from "svelte";
    import { page } from '$app/stores';
    import {Loader2Icon, File as FileIcon, Folder, UserRound, Upload, MousePointerClick, Trash2, CircleChevronLeft, House} from 'lucide-svelte'
    import { Separator } from "$lib/components/ui/separator/index.js";
	import { Progress } from "$lib/components/ui/progress/index.js";
    import * as Dialog from "$lib/components/ui/dialog/index.js";
	import Button from "$lib/components/ui/button/button.svelte";
	import Input from "$lib/components/ui/input/input.svelte";
	import Alert from "$lib/customComponents/Alert.svelte";
    import * as Breadcrumb from "$lib/components/ui/breadcrumb/index.js";
	import { toast, Toaster } from "svelte-sonner";
    import { authenticate, authorize, getInfo } from "$lib/auth/info";
	import { goto } from "$app/navigation";

    let currentPath = $derived($page.url.pathname.replace("/", ""));
    let breadcrumb = $derived(currentPath.split("/").slice(0, currentPath.split("/").length-1))
    let currentSources: {folders: any[], files: any[], canSee: boolean, canDownload: boolean, canUpload: boolean, canDelete: boolean, totalSize: number} = $state({
        folders: [],
        files: [],
        canSee: true,
        canDownload: true,
        canUpload: false,
        canDelete: false,
        totalSize: 0
    })
    let isFetching: boolean = $state(false);


    async function getSources(){
        isFetching = true;
        try {
            let result = await fetch(rootApi + "api/getSources/" + encodeURIComponent(currentPath));
            currentSources = await result.json();        
            isFetching = false;
            selectedDeleteSources.files = [];
            selectedDeleteSources.folders = [];
        } catch (error) {

        }
    }

    function convertByteFormat(bytes: number){
        if(bytes < 1024) return bytes + " B";
        else if(bytes < 1024 * 1024) return (bytes / 1024).toFixed(2) + " KB";
        else if(bytes < 1024 * 1024 * 1024) return (bytes / (1024 * 1024)).toFixed(2) + " MB";
        else return (bytes / (1024 * 1024 * 1024)).toFixed(2) + " GB";
    }

    let isOpenUpload = $state(false);
    function openUpload(){
        isOpenUpload = true;
    }
    let files = $state<FileList | null>();
    let filesArray = $derived(files ? Array.from(files) : []);
    let totalSize = $state(0);
    let uploadedSize =  $state(0);
    let uploadedFiles = $state(0);
    let startTime = 0;

    let lastUpdateTime = 0;
    let uploadSpeed = $state(0);
    let lastUploadedSize = 0;
    $effect(()=>{
        totalSize = files ? Array.from(files).reduce((sum, f) => sum + f.size, 0) : 0
        uploadedSize = 0;
        uploadedFiles = 0;
    })

    async function startUpload(){
        function connectWebSocket(url: string) : Promise<WebSocket> {
            return new Promise((resolve, reject) => {
                const socket = new WebSocket(url);

                socket.onopen = () => resolve(socket);
                socket.onerror = (err) => reject(err);
            });
        }

        let wsProtocol = window.location.protocol === "https:" ? "wss:" : "ws:";
        let wsUrl = wsProtocol + "//" + window.location.host + "/ws";
        const pendingResolvers: Record<number, (data: any) => void> = {};
    
        let socket: WebSocket;
        try{
            socket = await connectWebSocket(`ws://localhost:9090/__/ws?path=${currentPath}`);
        }catch{
            return;
        }
        
        socket.onmessage = (event) => {
            const ret = JSON.parse(event.data);
            
            switch (ret.event){
                case EVENTS.UPLOAD_START:
                    pendingResolvers[ret.event](ret.data);
                    delete pendingResolvers[ret.event];
                    break;
            }
        };
    
        
    
        function waitForEvent(event: number): Promise<any> {
            return new Promise((resolve) => {
                pendingResolvers[event] = resolve;
            });
        }
    
        function readChunkAsArrayBuffer(file: File, offset: number, chunkSize: number): Promise<ArrayBuffer> {
            return new Promise((resolve, reject) => {
                const reader = new FileReader();
                const slice = file.slice(offset, offset + chunkSize);
    
                reader.onload = () => {
                    if (reader.result instanceof ArrayBuffer) {
                        resolve(reader.result);
                    } else {
                        reject(new Error('Failed to read chunk as ArrayBuffer.'));
                    }
                };
                reader.onerror = reject;
    
                reader.readAsArrayBuffer(slice);
            });
        }
        
        
        if (files == null) return;
        let error = false;
        uploadedSize = 0;
        uploadedFiles = 0
        const chunkSize = 16 * 1024; // 16KB

        startTime = performance.now();
        lastUpdateTime = startTime;
        lastUploadedSize = 0;
        for (const file of files){
            let offset = 0;
            
            socket.send(JSON.stringify({
                event: EVENTS.UPLOAD_START, 
                data:{
                    name: file.name, 
                    path: currentPath, 
                    size: file.size
                }
                }) 
            )

            await waitForEvent(EVENTS.UPLOAD_START);
            while (offset < file.size) {
                try {
                    const chunk = await readChunkAsArrayBuffer(file, offset, chunkSize);
                    socket.send(chunk);
                    offset += chunk.byteLength;
                    uploadedSize += chunk.byteLength;

                    const now = performance.now();
                    const elapsedSinceLast = (now - lastUpdateTime) / 1000;
                    if (elapsedSinceLast >= 0.5) { 
                        const bytesSinceLast = uploadedSize - lastUploadedSize;
                        uploadSpeed = bytesSinceLast / elapsedSinceLast; 
                        lastUpdateTime = now;
                        lastUploadedSize = uploadedSize;
                    }
                } catch {
                    error = true
                    break; 
                }
            }

            socket.send(JSON.stringify({
                event: EVENTS.UPLOAD_COMPLETED, 
                data: ""}) 
            )

            if (error) {
                toast.error("Upload failed", {
                    duration: 5000
                })
                break;
            }
            uploadedFiles += 1;
            uploadSpeed = 0;
        }
        toast.success("Uploaded", {
            duration: 5000
        })
        await getSources();
    }

    let newDirName = $state("")
    async function makeDirectory(){
        try {
            let result = await fetch(rootApi+"api/makeDirectory", {
                method: "POST",
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({relative_path: currentPath, name: newDirName})
            });

            if (result.ok){
                toast.success("Created", {
                    duration: 5000
                })
            }else{
                toast.error("Could not create new directory", {
                    duration: 5000
                })
            }
        } catch (error) {
            toast.error("Could not create new directory", {
                duration: 5000
            })
        }
        await getSources();
    }

    let isSelect = $state(false);
    function onSelect(){
        isSelect = !isSelect;
        if (!isSelect) {
            selectedDeleteSources.files = [];
            selectedDeleteSources.folders = [];
        }
    }

    let selectedDeleteSources = $state({
        relative_path: "",
        folders: [],
        files: []
    })

    async function deleteSources(){
        selectedDeleteSources.relative_path = currentPath;
        try {
            let result = await fetch(rootApi+"api/deleteSources", {
                method: "POST",
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify(selectedDeleteSources)
            });

            toast.success("Deleted", {
                duration: 5000
            })
        } catch (error) {
            toast.error("Could not delete sources", {
                duration: 5000
            })
        }
        await getSources();
    }

    let isLogin = $state(false);
    let isAdmin = $state(false);
    let username = $state("");
    let isOpenUser = $state(false);

    async function checkAuth(){
        if (isLogin = await authenticate()){
            username = await getInfo();
            isAdmin = await authorize();
        }
    }

    let loginForm = $state({
        username: "",
        password: ""
    });
    
    function openUser(){
        isOpenUser = true;
    }

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
                await getSources();
                await checkAuth();
                isOpenUser = false;
            }else{
                toast.error("Invalid username or password", {
                    duration: 5000
                })
            }
        }catch{

        }
        isLogining = false;
    }

    async function logout(){
        try{
            let result = await fetch(rootApi + "api/logout",{
                method: "GET",
            })
            await getSources();
            await checkAuth();
        }catch{

        }
    }

    onMount(async () => {
        await getSources();
        await checkAuth();
    })

    $effect(() => {
        getSources();
    })

    // $inspect(breadcrumb)
</script>

<svelte:head>
	<title>LiliumFS Home Page</title>
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

<Dialog.Root bind:open={isOpenUpload}>
    <Dialog.Content class="">
        <Dialog.Header>
            <Dialog.Title>Upload files</Dialog.Title>
        </Dialog.Header>
        <Dialog.Root>
            <Dialog.Trigger>
                <Button variant="secondary">Make new directory</Button>
            </Dialog.Trigger>
            <Dialog.Content class="justify-baseline">
                <Dialog.Header>
                    <Dialog.Title>Enter name for new directory</Dialog.Title>
                </Dialog.Header>
                <Input bind:value={newDirName}/>
                <Button variant="secondary" onclick={async ()=>{await makeDirectory()}}>Make</Button>
            </Dialog.Content>
        </Dialog.Root>

        <Button variant="secondary"><input bind:files type="file" multiple class="w-full" /></Button>
        {#if filesArray.length > 0}
        <div class="flex flex-col gap-2 overflow-auto max-h-96">
            {#each filesArray as file}
                <p>{file.name}</p>
            {/each}
        </div>
            <div class="flex justify-between">
                <p>{uploadedFiles}/{filesArray.length}</p>
                <div class="flex gap-4">
                    <p>{(uploadSpeed / 1024 / 1024).toFixed(2)} MB/s</p>
                    <p>{(uploadedSize/totalSize * 100).toFixed(2)}%</p>
                </div>
            </div>
            <Progress value={(uploadedSize/totalSize * 100)} max={100} class="" />
        {/if}
        
        <Button variant="secondary" onclick={startUpload}>Start</Button>
    </Dialog.Content>
</Dialog.Root>

<!-- user panel -->
<Dialog.Root bind:open={isOpenUser}>
    <Dialog.Content class="">
        <Dialog.Header>
            {#if isLogin}
                <Dialog.Title>Welcome, {username}</Dialog.Title>
            {:else}
                <Dialog.Title>Login</Dialog.Title>
            {/if}
        </Dialog.Header>
        
        {#if isLogin}
            <div class="flex gap-2">
                <Button variant="ghost" onclick={async ()=>{ await logout()}}>Logout</Button>
                {#if isAdmin}
                    <Button variant="secondary" onclick={()=>{goto("/__/admin/")}}>Admin panel</Button>
                {/if}
            </div>
        {:else}
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
        {/if}
        
    </Dialog.Content>
</Dialog.Root>

<div class="w-screen h-screen flex flex-col justify-center items-center">
    <div class="w-2/3">
        <div class="flex mt-4 gap-2 transition-all ">
            <Button variant="secondary" onclick={openUser} class="transition-all">
                <UserRound/>
                <span class="hidden sm:inline">
                    {#if isLogin}
                        {username}
                    {:else}
                        Login
                    {/if}
                </span>
            </Button>
            {#if currentSources.canUpload}
                <Button variant="secondary" onclick={openUpload} class="transition-all">
                    <Upload/>
                    <span class="hidden sm:inline">Upload</span>
                </Button>
            {/if}
            <Button variant="secondary" onclick={onSelect} class="transition-all">
                <MousePointerClick/>
                <span class="hidden sm:inline">Select</span>
            </Button>
            {#if isSelect}
                {#if currentSources.canDelete}
                    <Alert bindFunction={deleteSources} trigger_title="Delete">
                        {#snippet icon()}
                            <Trash2/>
                        {/snippet}
                    </Alert>
                {/if}
            {/if}
        </div>
    
        <Breadcrumb.Root class="my-4">
            <Breadcrumb.List>
                <Breadcrumb.Item>
                    <Breadcrumb.Link href={"/"+breadcrumb.slice(0, breadcrumb.length-1).join("/")}><CircleChevronLeft/></Breadcrumb.Link>
                </Breadcrumb.Item>
                
                <Breadcrumb.Item>
                    <Breadcrumb.Link href="/"><House/></Breadcrumb.Link>
                </Breadcrumb.Item>
                
                {#each breadcrumb as path, index}
                    <Breadcrumb.Item class="bg-primary-foreground px-2 py-0.5 rounded-sm">
                        <Breadcrumb.Link class="text-md" href={"/"+breadcrumb.slice(0, index+1).join("/")}>{decodeURIComponent(path)}</Breadcrumb.Link>
                    </Breadcrumb.Item>
                    
                {/each}
            </Breadcrumb.List>
        </Breadcrumb.Root>
    </div>
    <div class="w-full flex-1 overflow-auto flex justify-center">
        <div class="w-2/3">
            
            {#if isFetching}
                <div class="flex justify-center items-center h-full">
                    <Loader2Icon class="animate-spin"/>
                </div>
            {:else}
                <div class=" flex flex-col m-auto w-full">
                    {#each currentSources.folders as folder}
                    <div role="none" class="flex flex-1 items-center break-words gap-2">
                        {#if isSelect}
                            <input type="checkbox" name="path" value={folder.name} bind:group={selectedDeleteSources.folders}>
                        {/if}
                        <a href={encodeURIComponent(folder.name)} class="w-full break-words p-2 rounded-sm flex items-center justify-between gap-2 hover:bg-accent flex-wrap">
                            <div class="flex items-center gap-2">
                                <Folder class="shrink-0"/>
                                <span>{folder.name}</span>
                            </div>
                            {#if folder.time}
                                <div class="flex items-center gap-2 text-sm">                   
                                    <span class="text-muted-foreground">{(new Date(folder.time)).toLocaleString()}</span>
                                </div>
                            {/if}
                        </a>
                    </div>
                    <Separator class="m-0"/>
                    {/each}
        
                    {#each currentSources.files as file}
                    <div role="none" class="flex flex-1 items-center break-words gap-2">
                        {#if isSelect}
                            <input type="checkbox" name="path" value={file.name} bind:group={selectedDeleteSources.files}>
                        {/if}
                        <a href={rootUrl+currentPath+encodeURIComponent(file.name)} target="_blank" class="w-full break-words p-2 rounded-sm flex items-center justify-between gap-2 hover:bg-accent flex-wrap">
                            <div class="flex items-center gap-2">
                                <FileIcon class="shrink-0"/>
                                <span>{file.name}</span>
                            </div>
                            <div class="flex items-center gap-2 text-sm">
                                <span class="text-muted-foreground">{convertByteFormat(file.size)}</span>                      
                                <span class="text-muted-foreground">{(new Date(file.time)).toLocaleString()}</span>
                            </div>
                        </a>
                    </div>
                        <Separator class="m-0"/>
                    {/each}
                </div>
            {/if}
        </div>
    </div>
    <div class="h-10 w-full border-t-1 border-t-secondary bg-sidebar-accent/80">
        <div class="w-full h-full flex items-center justify-between flex-wrap gap-2 px-4 py-2">
                    
            <p class="text-md text-muted-foreground">{currentSources.folders.length} folders, {currentSources.files.length} files, {convertByteFormat(currentSources.totalSize)}</p>
        </div>
    </div>
</div>



<style>
    .break-words{
        word-break: break-word;
    }
</style>