<script lang="ts">
    import * as Dialog from "$lib/components/ui/dialog/index.js";
    import * as DropdownMenu from "$lib/components/ui/dropdown-menu/index.js";
    import { Input } from "$lib/components/ui/input/index.js";
    import { Button } from "$lib/components/ui/button/index.js";
    import { Checkbox } from "$lib/components/ui/checkbox/index.js";
    import * as RadioGroup from "$lib/components/ui/radio-group/index.js";
    import { Label } from "$lib/components/ui/label/index.js";
    import { toast } from "svelte-sonner";

	import { rootApi } from "$lib/stores/globalValues";
	import { onMount } from "svelte";
    import { Folder, File, Eye, Lock, EyeOff} from 'lucide-svelte';
    import Add from "./Add.svelte"
	import type { file, folder, virtual } from "$lib/types/types";
	import RuleDropDown from "./RuleDropDown.svelte";
	import Separator from "$lib/components/ui/separator/separator.svelte";

    let sources: {folders: folder[], files: file[], virtual: virtual[]}= $state({
        folders: [],
        files: [],
        virtual: []
    });
    async function getSources(){
        try {
            const result = await fetch(rootApi+"admin/api/getSources");
            sources = await result.json();
        } catch (error) {
            
        }
    }

    let isOpenAdd: boolean = $state(false);
    async function afterAdd() {
        isOpenAdd = false;
        isOpenAddVirtualChild = false;
        await getSources(); 
    }

    let isOpenChange: boolean = $state(false);
    let initChangeSource = {
        name: "",
        src: "",
        type: "folder",
        canSee: "",
        canUpload: "",
        canDelete: "",
        canDownload: "",
    }

    let changingSource = $state({
        name: "",
        src: "",
        type: "folder",
        canSee: "",
        canUpload: "",
        canDelete: "",
        canDownload: "",
    })
    
    let accessRules = ["Anyone", "No one", "Any account"];
    interface account {
		username: string;
	}

	let accounts: account[] = $state([]);

	async function getAccounts() {
		try {
			let result = await fetch(rootApi + 'admin/api/getAccounts');
			accounts = await result.json();
		} catch (error) {}
	}
    
    async function openChange(changedata: any, type: "folder" | "file"){
        isOpenChange = true;
        initChangeSource = changedata;
        initChangeSource.type = type;
        await getAccounts()
        changingSource = JSON.parse(JSON.stringify(initChangeSource));
    }

    async function change(){
        try {
            let result = await fetch(rootApi+"admin/api/changeSource", {
                method: "POST",
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify(changingSource)
            });

            if (!result.ok) {
                const message = await result.text();
                toast.warning(message, {
                    duration: 5000
                })
            }else{
                toast.success("Changed", {
                    duration: 5000
                })
                await getSources();
                isOpenChange = false;
            }
        } catch (error) {
            toast.error("Unknown error", {
                duration: 5000
            })
        }
    }

    let isOpenDelete: boolean = $state(false);
    let deleteSource = $state({
        src: "",
        type: "folder"
    })
    function openDelete(src: string, type: "folder" | "file"){
        isOpenDelete = true;
        deleteSource.src = src;
        deleteSource.type = type;
    }

    async function onDelete(){
        try {
            let result = await fetch(rootApi+"admin/api/deleteSource", {
                method: "POST",
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify(deleteSource)
            });

            if (result.ok){
                toast.success("Deleted", {
                    duration: 5000
                })
                isOpenDelete = false;
            }
        } catch (error) {
            toast.error("Unknown error", {
                duration: 5000
            })
        }
        isOpenDelete = false;
        await getSources();
    }


    /**
     * virtual root
     */
    let newVirtualRootName = $state("");
    //let selectedVirtualRootName = $state("");
    let isOpenAddVirtualRoot = $state(false);
    let isOpenAddVirtualChild = $state(false);
    async function addVirtualRoot(){
        try {
            let result = await fetch(rootApi+"admin/api/addVirtualRoot", {
                method: "POST",
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({name: newVirtualRootName})
            });
            
            if (!result.ok) {
                const message = await result.text();
                toast.warning(message, {
                    duration: 5000
                })
            }else{
                toast.success("Created new virtual root", {
                    duration: 5000
                })
                await getSources();
                isOpenAddVirtualRoot = false;
            }
        } catch (error) {
            toast.error("Unknown error", {
                duration: 5000
            })
        }
    }


    let initChangeVirtualRoot = {
        oldName: "",
        newName: "",
        canSee: "",
        canDownload: "",
    }

    let changingVirtualRoot = $state({
        oldName: "",
        newName: "",
        canSee: "",
        canDownload: "",
    })

    let isOpenChangeVirtualRoot: boolean = $state(false);
    async function openChangeVirtualRoot(changedata: any){
        isOpenChangeVirtualRoot = true;
        initChangeVirtualRoot.oldName = changedata.name;
        initChangeVirtualRoot.newName = changedata.name;
        initChangeVirtualRoot.canSee = changedata.canSee;
        initChangeVirtualRoot.canDownload = changedata.canDownload;
        await getAccounts()
        changingVirtualRoot = JSON.parse(JSON.stringify(initChangeVirtualRoot)); 
    }

    async function changeVirtualRoot(){
        try {
            let result = await fetch(rootApi+"admin/api/changeVirtualRoot", {
                method: "POST",
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify(changingVirtualRoot)
            });

            if (!result.ok) {
                const message = await result.text();
                toast.warning(message, {
                    duration: 5000
                })
            }else{
                toast.success("Changed", {
                    duration: 5000
                })
                await getSources();
                isOpenChangeVirtualRoot = false;
            }
        } catch (error) {
            toast.error("Unknown error", {
                duration: 5000
            })
        }
    }

    let isOpenDeleteVirtualRoot: boolean = $state(false);
    let deleteVirtualRoot = $state({
        name: ""
    })
    function openDeleteVirtualRoot(name: string){
        isOpenDeleteVirtualRoot = true;
        deleteVirtualRoot.name = name;
        console.log(deleteVirtualRoot.name);
    }

    async function onDeleteVirtualRoot(){
        try {
            let result = await fetch(rootApi+"admin/api/deleteVirtualRoot", {
                method: "POST",
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify(deleteVirtualRoot)
            });

            if (result.ok){
                toast.success("Deleted", {
                    duration: 5000
                })
                isOpenDeleteVirtualRoot = false;
            }
        } catch (error) {
            toast.error("Unknown error", {
                duration: 5000
            })
        }
        isOpenDeleteVirtualChild = false;
        await getSources();
    }

    /**
     * virtual child
     */
    let initChangeVirtualChild = {
        virtualRootName: "",
        name: "",
        src: "",
        type: "folder",
        canSee: "",
        canUpload: "",
        canDelete: "",
        canDownload: "",
    }

    let changingVirtualChild = $state({
        virtualRootName: "",
        name: "",
        src: "",
        type: "folder",
        canSee: "",
        canUpload: "",
        canDelete: "",
        canDownload: "",
    })

    let isOpenChangeVirtualChild: boolean = $state(false);
    async function openChangeVirtualChild(changedata: any, type: "folder" | "file", virtualRootName: string){
        isOpenChangeVirtualChild = true;
        initChangeVirtualChild = changedata;
        initChangeVirtualChild.type = type;
        initChangeVirtualChild.virtualRootName = virtualRootName;
        await getAccounts()
        changingVirtualChild = JSON.parse(JSON.stringify(initChangeVirtualChild)); 
    }

    async function changeVirtualChild(){
        try {
            let result = await fetch(rootApi+"admin/api/changeVirtualChild", {
                method: "POST",
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify(changingVirtualChild)
            });

            if (!result.ok) {
                const message = await result.text();
                toast.warning(message, {
                    duration: 5000
                })
            }else{
                toast.success("Changed", {
                    duration: 5000
                })
                await getSources();
                isOpenChangeVirtualChild = false;
            }
        } catch (error) {
            toast.error("Unknown error", {
                duration: 5000
            })
        }
    }

    let isOpenDeleteVirtualChild: boolean = $state(false);
    let deleteVirtualChild = $state({
        virtualRootName: "",
        src: "",
        type: "folder"
    })
    function openDeleteVirtualChild(src: string, type: "folder" | "file", virtualRootName: string){
        isOpenDeleteVirtualChild = true;
        deleteVirtualChild.src = src;
        deleteVirtualChild.type = type;
        deleteVirtualChild.virtualRootName = virtualRootName;
    }

    async function onDeleteVirtualChild(){
        try {
            let result = await fetch(rootApi+"admin/api/deleteVirtualChild", {
                method: "POST",
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify(deleteVirtualChild)
            });

            if (result.ok){
                toast.success("Deleted", {
                    duration: 5000
                })
                isOpenDeleteVirtualChild = false;
            }
        } catch (error) {
            toast.error("Unknown error", {
                duration: 5000
            })
        }
        isOpenDeleteVirtualChild = false;
        await getSources();
    }

    
    
    onMount(async ()=>{
        await getSources();
    })
    $inspect(sources)
</script>

<div class="px-4 pb-20">
    <h1>Sources</h1>
    <!-- sources -->
    <Dialog.Root bind:open={isOpenChange}>
      <Dialog.Content>
        <div class="break-words">
            <p class="text-muted-foreground mb-2">{changingSource.src}</p>
            <h2 class="mb-2">Source name (alias):</h2>
            <Input bind:value={changingSource.name} spellcheck=false class="mb-2"></Input>
            <div class="flex justify-between mt-5">
                <RuleDropDown label="Who can see" accounts={accounts} bind:value={changingSource.canSee}></RuleDropDown>
                <RuleDropDown label="Who can Download" accounts={accounts} bind:value={changingSource.canDownload}></RuleDropDown>
            </div>
            {#if changingSource.canUpload && changingSource.canDelete}    
            <div class="flex justify-between mt-5">
                <RuleDropDown label="Who can Upload" accounts={accounts} bind:value={changingSource.canUpload}></RuleDropDown>
                <RuleDropDown label="Who can Delete" accounts={accounts} bind:value={changingSource.canDelete}></RuleDropDown>
            </div>
            {/if}
            <div class="flex justify-end items-center mt-4">
                <Button variant="secondary" disabled={JSON.stringify(initChangeSource) === JSON.stringify(changingSource)} onclick={async ()=>{await change()}}>Save</Button>
            </div>
        </div>
      </Dialog.Content>
    </Dialog.Root>

    <Dialog.Root bind:open={isOpenDelete}>
      <Dialog.Content>
        <div class="break-words">
            <p class="mb-2">Do you want to delete <span class="text-muted-foreground">{deleteSource.src}</span> ?</p>  
            <div class="flex justify-end items-center">
                <Button variant="destructive" onclick={async ()=>{await onDelete()}}>Yes</Button>
            </div>
        </div>
      </Dialog.Content>
    </Dialog.Root>

    <!-- virtual roots -->

    <Dialog.Root bind:open={isOpenChangeVirtualRoot}>
      <Dialog.Content>
        <div class="break-words">
            <h2 class="mb-2">Virtual root name:</h2>
            <Input bind:value={changingVirtualRoot.newName} spellcheck=false class="mb-2"></Input>
            <div class="flex justify-between mt-5">
                <RuleDropDown label="Who can see" accounts={accounts} bind:value={changingVirtualRoot.canSee}></RuleDropDown>
                <RuleDropDown label="Who can Download" accounts={accounts} bind:value={changingVirtualRoot.canDownload}></RuleDropDown>
            </div>
            <div class="flex justify-end items-center mt-4">
                <Button variant="secondary" disabled={JSON.stringify(initChangeVirtualRoot) === JSON.stringify(changingVirtualRoot)} onclick={async ()=>{await changeVirtualRoot()}}>Save</Button>
            </div>
        </div>
      </Dialog.Content>
    </Dialog.Root>

    <Dialog.Root bind:open={isOpenDeleteVirtualRoot}>
      <Dialog.Content>
        <div class="break-words">
            <p class="mb-2">Do you want to delete <span class="text-muted-foreground">{deleteVirtualRoot.name}</span> ?</p>  
            <div class="flex justify-end items-center">
                <Button variant="destructive" onclick={async ()=>{await onDeleteVirtualRoot()}}>Yes</Button>
            </div>
        </div>
      </Dialog.Content>
    </Dialog.Root>

    <!-- virtual child -->

    <Dialog.Root bind:open={isOpenChangeVirtualChild}>
      <Dialog.Content>
        <div class="break-words">
            <p class="text-muted-foreground mb-2">{changingVirtualChild.src}</p>
            <h2 class="mb-2">Source name (alias):</h2>
            <Input bind:value={changingVirtualChild.name} spellcheck=false class="mb-2"></Input>
            <div class="flex justify-between mt-5">
                <RuleDropDown label="Who can see" accounts={accounts} bind:value={changingVirtualChild.canSee}></RuleDropDown>
                <RuleDropDown label="Who can Download" accounts={accounts} bind:value={changingVirtualChild.canDownload}></RuleDropDown>
            </div>
            {#if changingVirtualChild.canUpload && changingVirtualChild.canDelete}    
            <div class="flex justify-between mt-5">
                <RuleDropDown label="Who can Upload" accounts={accounts} bind:value={changingVirtualChild.canUpload}></RuleDropDown>
                <RuleDropDown label="Who can Delete" accounts={accounts} bind:value={changingVirtualChild.canDelete}></RuleDropDown>
            </div>
            {/if}
            <div class="flex justify-end items-center mt-4">
                <Button variant="secondary" disabled={JSON.stringify(initChangeVirtualChild) === JSON.stringify(changingVirtualChild)} onclick={async ()=>{await changeVirtualChild()}}>Save</Button>
            </div>
        </div>
      </Dialog.Content>
    </Dialog.Root>

    <Dialog.Root bind:open={isOpenDeleteVirtualChild}>
      <Dialog.Content>
        <div class="break-words">
            <p class="mb-2">Do you want to delete <span class="text-muted-foreground">{deleteVirtualChild.src}</span> ?</p>  
            <div class="flex justify-end items-center">
                <Button variant="destructive" onclick={async ()=>{await onDeleteVirtualChild()}}>Yes</Button>
            </div>
        </div>
      </Dialog.Content>
    </Dialog.Root>

    
    
    <div class="flex gap-2">
        <Dialog.Root bind:open={isOpenAdd}>
            <Dialog.Trigger>
                <Button variant="secondary" class="my-4">Add Sources</Button>
            </Dialog.Trigger>
            <Dialog.Content class="h-3/4">
                <Add {afterAdd}/>
            </Dialog.Content>
        </Dialog.Root>
    </div>
    
    <div class="flex flex-col">
        {#each sources.folders as folder}
            <DropdownMenu.Root>
            <DropdownMenu.Trigger>
                <div class="flex items-center mb-1 p-2 cursor-pointer rounded-sm hover:bg-accent">
                    <div role="none" class="flex flex-1 items-center " onclick={async ()=>{}}>
                        <Folder/>
                        <span class="ml-2 flex-1 break-words  text-start">{folder.name}<span class="text-muted-foreground">{" ("+folder.src+")"}</span></span>
                        {#if folder.canSee == "Anyone"}
                            <Eye class="ml-2"/>  
                        {:else if folder.canSee == "No one"}
                            <EyeOff class="ml-2"/>
                        {:else}
                            <Lock class="ml-2"/>
                        {/if}
                    </div>
                </div>
            </DropdownMenu.Trigger>
            <DropdownMenu.Content align="start">
                <DropdownMenu.Group>
                    <DropdownMenu.Item onclick={()=>{openChange(folder, "folder")}}>Change</DropdownMenu.Item>
                    <DropdownMenu.Item onclick={()=>{openDelete(folder.src, "folder")}}>Delete</DropdownMenu.Item>
                </DropdownMenu.Group>
            </DropdownMenu.Content>
            </DropdownMenu.Root>
        {/each}
        {#each sources.files as file}
            <DropdownMenu.Root>
                <DropdownMenu.Trigger>
                    <div class="flex items-center mb-1 p-2 cursor-pointer rounded-sm hover:bg-accent">
                        <div role="none" class="flex flex-1 items-center " onclick={async ()=>{}}>
                            <File/>
                            <span class="ml-2 flex-1 break-words text-start">{file.name}<span class="text-muted-foreground">{" ("+file.src+")"}</span> </span>
                            {#if file.canSee == "Anyone"}
                                <Eye class="ml-2"/>  
                            {:else if file.canSee == "No one"}
                                <EyeOff class="ml-2"/>
                            {:else}
                                <Lock class="ml-2"/>
                            {/if}
                        </div>
                    </div>
                </DropdownMenu.Trigger>
                <DropdownMenu.Content align="start">
                    <DropdownMenu.Group>
                        <DropdownMenu.Item onclick={()=>{openChange(file, "file")}}>Change</DropdownMenu.Item>
                        <DropdownMenu.Item onclick={()=>{openDelete(file.src, "file")}}>Delete</DropdownMenu.Item>
                    </DropdownMenu.Group>
                </DropdownMenu.Content>
            </DropdownMenu.Root>
        {/each}
    </div>

    <Separator class="mt-2"/>

    <h1 class="mt-2">Virtual roots</h1>
    <Dialog.Root bind:open={isOpenAddVirtualRoot}>
        <Dialog.Trigger>
            <Button variant="secondary" class="my-4">Add virtual root</Button>
        </Dialog.Trigger>
        <Dialog.Content class="">
            <Dialog.Header>
                <Dialog.Title>Enter name for new virtual root</Dialog.Title>
            </Dialog.Header>
            <Input bind:value={newVirtualRootName}/>
            <Button variant="secondary" onclick={async ()=>{await addVirtualRoot()}}>Add</Button>
        </Dialog.Content>
    </Dialog.Root>
    <div class="flex flex-col">
        {#each sources.virtual as virtual}
            <ul>
                <li>
                    <DropdownMenu.Root>
                        <DropdownMenu.Trigger class="w-full">
                            <div class="flex items-center mb-1 p-2 cursor-pointer rounded-sm hover:bg-accent">
                                <div role="none" class="flex flex-1 items-center " onclick={async ()=>{}}>
                                    <Folder/>
                                    <span class="ml-2 flex-1 break-words text-start">{virtual.name}</span>
                                    {#if virtual.canSee == "Anyone"}
                                            <Eye class="ml-2"/>  
                                        {:else if virtual.canSee == "No one"}
                                            <EyeOff class="ml-2"/>
                                        {:else}
                                            <Lock class="ml-2"/>
                                        {/if}
                                </div>
                            </div>
                        </DropdownMenu.Trigger>
                        <DropdownMenu.Content align="start">
                            <DropdownMenu.Group>
                                <DropdownMenu.Item onSelect={(e)=>{e.preventDefault()}}>
                                    <Dialog.Root bind:open={isOpenAddVirtualChild}>
                                        <Dialog.Trigger>
                                            Add Sources
                                        </Dialog.Trigger>
                                        <Dialog.Content class="h-3/4">
                                            <Add afterAdd={afterAdd} isVirtual={true} virtualRootName={virtual.name}/>
                                        </Dialog.Content>
                                    </Dialog.Root>
                                </DropdownMenu.Item>
                                <DropdownMenu.Item onclick={()=>{openChangeVirtualRoot(virtual)}}>Change</DropdownMenu.Item>
                                <DropdownMenu.Item onclick={()=>{openDeleteVirtualRoot(virtual.name)}}>Delete</DropdownMenu.Item>
                            </DropdownMenu.Group>
                        </DropdownMenu.Content>
                    </DropdownMenu.Root>
                </li>
                <ul class="ml-5 pl-4 border-l border-gray-300">
                    {#each virtual.folders as folder}
                        <li class="relative before:content-[''] before:absolute before:top-5 before:-left-4 before:w-4 before:border-t before:border-gray-300">
                            <DropdownMenu.Root>
                            <DropdownMenu.Trigger class="w-full">
                                <div class="flex items-center mb-1 p-2 cursor-pointer rounded-sm hover:bg-accent">
                                    <div role="none" class="flex flex-1 items-center " onclick={async ()=>{}}>
                                        <Folder/>
                                        <span class="ml-2 flex-1 break-words  text-start">{folder.name}<span class="text-muted-foreground">{" ("+folder.src+")"}</span></span>
                                        {#if folder.canSee == "Anyone"}
                                            <Eye class="ml-2"/>  
                                        {:else if folder.canSee == "No one"}
                                            <EyeOff class="ml-2"/>
                                        {:else}
                                            <Lock class="ml-2"/>
                                        {/if}
                                    </div>
                                </div>
                            </DropdownMenu.Trigger>
                            <DropdownMenu.Content align="start">
                                <DropdownMenu.Group>
                                    <DropdownMenu.Item onclick={()=>{openChangeVirtualChild(folder, "folder", virtual.name)}}>Change</DropdownMenu.Item>
                                    <DropdownMenu.Item onclick={()=>{openDeleteVirtualChild(folder.src, "folder", virtual.name)}}>Delete</DropdownMenu.Item>
                                </DropdownMenu.Group>
                            </DropdownMenu.Content>
                            </DropdownMenu.Root>
                        </li>
                    {/each}
                    {#each virtual.files as file}
                        <li class="relative before:content-[''] before:absolute before:top-5 before:-left-4 before:w-4 before:border-t before:border-gray-300">
                            <DropdownMenu.Root>
                                <DropdownMenu.Trigger class="w-full">
                                    <div class="flex items-center mb-1 p-2 cursor-pointer rounded-sm hover:bg-accent">
                                        <div role="none" class="flex flex-1 items-center " onclick={async ()=>{}}>
                                            <File/>
                                            <span class="ml-2 flex-1 break-words text-start">{file.name}<span class="text-muted-foreground">{" ("+file.src+")"}</span> </span>
                                            {#if file.canSee == "Anyone"}
                                                <Eye class="ml-2"/>  
                                            {:else if file.canSee == "No one"}
                                                <EyeOff class="ml-2"/>
                                            {:else}
                                                <Lock class="ml-2"/>
                                            {/if}
                                        </div>
                                    </div>
                                </DropdownMenu.Trigger>
                                <DropdownMenu.Content align="start">
                                    <DropdownMenu.Group>
                                        <DropdownMenu.Item onclick={()=>{openChangeVirtualChild(file, "file", virtual.name)}}>Change</DropdownMenu.Item>
                                        <DropdownMenu.Item onclick={()=>{openDeleteVirtualChild(file.src, "file", virtual.name)}}>Delete</DropdownMenu.Item>
                                    </DropdownMenu.Group>
                                </DropdownMenu.Content>
                            </DropdownMenu.Root>
                        </li>
                    {/each}
                </ul>
            </ul>
        {/each}
    </div>
</div>

<style>
    .break-words{
        word-break: break-word;
    }

    /* .tree{
        margin-left: 5px;
        padding-left: 4px;
        border-left: 1px;
        border-color: aliceblue;
    } */

    .tree li:last-child {
        height: 1px;
    }
    
</style>


