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
	import type { file, dir, vd } from "$lib/types/types";
	import RuleDropDown from "./RuleDropDown.svelte";
	import Separator from "$lib/components/ui/separator/separator.svelte";

    let sources: {dirs: dir[], files: file[], vds: vd[]}= $state({
        dirs: [],
        files: [],
        vds: []
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
        type: "dir",
        can_see: "",
        can_upload: "",
        can_delete: "",
        can_download: "",
    }

    let changingSource = $state({
        name: "",
        src: "",
        type: "dir",
        can_see: "",
        can_upload: "",
        can_delete: "",
        can_download: "",
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
    
    async function openChange(changedata: any, type: "dir" | "file"){
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
        type: "dir"
    })
    function openDelete(src: string, type: "dir" | "file"){
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
     * vd root
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
                toast.success("Created new vd root", {
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
        old_name: "",
        new_name: "",
        can_see: "",
        can_download: "",
    }

    let changingVirtualRoot = $state({
        old_name: "",
        new_name: "",
        can_see: "",
        can_download: "",
    })

    let isOpenChangeVirtualRoot: boolean = $state(false);
    async function openChangeVirtualRoot(changedata: any){
        isOpenChangeVirtualRoot = true;
        initChangeVirtualRoot.old_name = changedata.name;
        initChangeVirtualRoot.new_name = changedata.name;
        initChangeVirtualRoot.can_see = changedata.can_see;
        initChangeVirtualRoot.can_download = changedata.can_download;
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
     * vd child
     */
    let initChangeVirtualChild = {
        vd_name: "",
        name: "",
        src: "",
        type: "dir",
        can_see: "",
        can_upload: "",
        can_delete: "",
        can_download: "",
    }

    let changingVirtualChild = $state({
        vd_name: "",
        name: "",
        src: "",
        type: "dir",
        can_see: "",
        can_upload: "",
        can_delete: "",
        can_download: "",
    })

    let isOpenChangeVirtualChild: boolean = $state(false);
    async function openChangeVirtualChild(changedata: any, type: "dir" | "file", vd_name: string){
        isOpenChangeVirtualChild = true;
        initChangeVirtualChild = changedata;
        initChangeVirtualChild.type = type;
        initChangeVirtualChild.vd_name = vd_name;
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
        vd_name: "",
        src: "",
        type: "dir"
    })
    function openDeleteVirtualChild(src: string, type: "dir" | "file", vd_name: string){
        isOpenDeleteVirtualChild = true;
        deleteVirtualChild.src = src;
        deleteVirtualChild.type = type;
        deleteVirtualChild.vd_name = vd_name;
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
                <RuleDropDown label="Who can see" accounts={accounts} bind:value={changingSource.can_see}></RuleDropDown>
                <RuleDropDown label="Who can download" accounts={accounts} bind:value={changingSource.can_download}></RuleDropDown>
            </div>
            {#if changingSource.can_upload && changingSource.can_delete}    
            <div class="flex justify-between mt-5">
                <RuleDropDown label="Who can upload" accounts={accounts} bind:value={changingSource.can_upload}></RuleDropDown>
                <RuleDropDown label="Who can delete" accounts={accounts} bind:value={changingSource.can_delete}></RuleDropDown>
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

    <!-- vd roots -->

    <Dialog.Root bind:open={isOpenChangeVirtualRoot}>
      <Dialog.Content>
        <div class="break-words">
            <h2 class="mb-2">Virtual root name:</h2>
            <Input bind:value={changingVirtualRoot.new_name} spellcheck=false class="mb-2"></Input>
            <div class="flex justify-between mt-5">
                <RuleDropDown label="Who can see" accounts={accounts} bind:value={changingVirtualRoot.can_see}></RuleDropDown>
                <RuleDropDown label="Who can download" accounts={accounts} bind:value={changingVirtualRoot.can_download}></RuleDropDown>
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

    <!-- vd child -->

    <Dialog.Root bind:open={isOpenChangeVirtualChild}>
      <Dialog.Content>
        <div class="break-words">
            <p class="text-muted-foreground mb-2">{changingVirtualChild.src}</p>
            <h2 class="mb-2">Source name (alias):</h2>
            <Input bind:value={changingVirtualChild.name} spellcheck=false class="mb-2"></Input>
            <div class="flex justify-between mt-5">
                <RuleDropDown label="Who can see" accounts={accounts} bind:value={changingVirtualChild.can_see}></RuleDropDown>
                <RuleDropDown label="Who can download" accounts={accounts} bind:value={changingVirtualChild.can_download}></RuleDropDown>
            </div>
            {#if changingVirtualChild.can_upload && changingVirtualChild.can_delete}    
            <div class="flex justify-between mt-5">
                <RuleDropDown label="Who can upload" accounts={accounts} bind:value={changingVirtualChild.can_upload}></RuleDropDown>
                <RuleDropDown label="Who can delete" accounts={accounts} bind:value={changingVirtualChild.can_delete}></RuleDropDown>
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
        {#each sources.dirs as dir}
            <DropdownMenu.Root>
            <DropdownMenu.Trigger>
                <div class="flex items-center mb-1 p-2 cursor-pointer rounded-sm hover:bg-accent">
                    <div role="none" class="flex flex-1 items-center " onclick={async ()=>{}}>
                        <Folder/>
                        <span class="ml-2 flex-1 break-words  text-start">{dir.name}<span class="text-muted-foreground">{" ("+dir.src+")"}</span></span>
                        {#if dir.can_see == "Anyone"}
                            <Eye class="ml-2"/>  
                        {:else if dir.can_see == "No one"}
                            <EyeOff class="ml-2"/>
                        {:else}
                            <Lock class="ml-2"/>
                        {/if}
                    </div>
                </div>
            </DropdownMenu.Trigger>
            <DropdownMenu.Content align="start">
                <DropdownMenu.Group>
                    <DropdownMenu.Item onclick={()=>{openChange(dir, "dir")}}>Change</DropdownMenu.Item>
                    <DropdownMenu.Item onclick={()=>{openDelete(dir.src, "dir")}}>Delete</DropdownMenu.Item>
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
                            {#if file.can_see == "Anyone"}
                                <Eye class="ml-2"/>  
                            {:else if file.can_see == "No one"}
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

    <h1 class="mt-2">Virtual Directories</h1>
    <Dialog.Root bind:open={isOpenAddVirtualRoot}>
        <Dialog.Trigger>
            <Button variant="secondary" class="my-4">Add Virtual Directory</Button>
        </Dialog.Trigger>
        <Dialog.Content class="">
            <Dialog.Header>
                <Dialog.Title>Enter name for new vd root</Dialog.Title>
            </Dialog.Header>
            <Input bind:value={newVirtualRootName}/>
            <Button variant="secondary" onclick={async ()=>{await addVirtualRoot()}}>Add</Button>
        </Dialog.Content>
    </Dialog.Root>
    <div class="flex flex-col">
        {#each sources.vds as vd}
            <ul>
                <li>
                    <DropdownMenu.Root>
                        <DropdownMenu.Trigger class="w-full">
                            <div class="flex items-center mb-1 p-2 cursor-pointer rounded-sm hover:bg-accent">
                                <div role="none" class="flex flex-1 items-center " onclick={async ()=>{}}>
                                    <Folder/>
                                    <span class="ml-2 flex-1 break-words text-start">{vd.name}</span>
                                    {#if vd.can_see == "Anyone"}
                                            <Eye class="ml-2"/>  
                                        {:else if vd.can_see == "No one"}
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
                                            <Add afterAdd={afterAdd} is_vd={true} vd_name={vd.name}/>
                                        </Dialog.Content>
                                    </Dialog.Root>
                                </DropdownMenu.Item>
                                <DropdownMenu.Item onclick={()=>{openChangeVirtualRoot(vd)}}>Change</DropdownMenu.Item>
                                <DropdownMenu.Item onclick={()=>{openDeleteVirtualRoot(vd.name)}}>Delete</DropdownMenu.Item>
                            </DropdownMenu.Group>
                        </DropdownMenu.Content>
                    </DropdownMenu.Root>
                </li>
                <ul class="ml-5 pl-4 border-l border-gray-300">
                    {#each vd.dirs as dir}
                        <li class="relative before:content-[''] before:absolute before:top-5 before:-left-4 before:w-4 before:border-t before:border-gray-300">
                            <DropdownMenu.Root>
                            <DropdownMenu.Trigger class="w-full">
                                <div class="flex items-center mb-1 p-2 cursor-pointer rounded-sm hover:bg-accent">
                                    <div role="none" class="flex flex-1 items-center " onclick={async ()=>{}}>
                                        <Folder/>
                                        <span class="ml-2 flex-1 break-words  text-start">{dir.name}<span class="text-muted-foreground">{" ("+dir.src+")"}</span></span>
                                        {#if dir.can_see == "Anyone"}
                                            <Eye class="ml-2"/>  
                                        {:else if dir.can_see == "No one"}
                                            <EyeOff class="ml-2"/>
                                        {:else}
                                            <Lock class="ml-2"/>
                                        {/if}
                                    </div>
                                </div>
                            </DropdownMenu.Trigger>
                            <DropdownMenu.Content align="start">
                                <DropdownMenu.Group>
                                    <DropdownMenu.Item onclick={()=>{openChangeVirtualChild(dir, "dir", vd.name)}}>Change</DropdownMenu.Item>
                                    <DropdownMenu.Item onclick={()=>{openDeleteVirtualChild(dir.src, "dir", vd.name)}}>Delete</DropdownMenu.Item>
                                </DropdownMenu.Group>
                            </DropdownMenu.Content>
                            </DropdownMenu.Root>
                        </li>
                    {/each}
                    {#each vd.files as file}
                        <li class="relative before:content-[''] before:absolute before:top-5 before:-left-4 before:w-4 before:border-t before:border-gray-300">
                            <DropdownMenu.Root>
                                <DropdownMenu.Trigger class="w-full">
                                    <div class="flex items-center mb-1 p-2 cursor-pointer rounded-sm hover:bg-accent">
                                        <div role="none" class="flex flex-1 items-center " onclick={async ()=>{}}>
                                            <File/>
                                            <span class="ml-2 flex-1 break-words text-start">{file.name}<span class="text-muted-foreground">{" ("+file.src+")"}</span> </span>
                                            {#if file.can_see == "Anyone"}
                                                <Eye class="ml-2"/>  
                                            {:else if file.can_see == "No one"}
                                                <EyeOff class="ml-2"/>
                                            {:else}
                                                <Lock class="ml-2"/>
                                            {/if}
                                        </div>
                                    </div>
                                </DropdownMenu.Trigger>
                                <DropdownMenu.Content align="start">
                                    <DropdownMenu.Group>
                                        <DropdownMenu.Item onclick={()=>{openChangeVirtualChild(file, "file", vd.name)}}>Change</DropdownMenu.Item>
                                        <DropdownMenu.Item onclick={()=>{openDeleteVirtualChild(file.src, "file", vd.name)}}>Delete</DropdownMenu.Item>
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


