<script lang="ts">
	import { rootApi } from '$lib/stores/globalValues';
	import { Input } from '$lib/components/ui/input/index.js';
	import { Button } from '$lib/components/ui/button/index.js';
	import * as Dialog from '$lib/components/ui/dialog/index.js';
	import * as DropdownMenu from "$lib/components/ui/dropdown-menu/index.js";
    import { Checkbox } from "$lib/components/ui/checkbox/index.js";
    import { Label } from "$lib/components/ui/label/index.js";

	import { toast } from "svelte-sonner";

    import { User, UserStar } from 'lucide-svelte';

	import { onMount } from 'svelte';
	import { Inspect } from 'lucide-svelte';

	interface account {
		username: string;
		password: string | null;
		isAdmin: boolean;
	}

	let accounts: account[] = $state([]);

	async function getAccounts() {
		try {
			let result = await fetch(rootApi + 'admin/api/getAccounts');
			accounts = await result.json();
		} catch (error) {
			toast.error("Unknown error", {
				duration: 5000
			})
		}
	}

	let isAddOpen = $state(false);
	let addAccountDialog: account = $state({
		username: '',
		password: '',
		isAdmin: false
	});

	let isChangeOpen = $state(false);
	let changeAccountDialog: account = $state({
		username: '',
		password: '',
		isAdmin: false
	});

	let isDeleteOpen = $state(false);
	let deleteAccountDialog: account = $state({
		username: '',
		password: '',
		isAdmin: false
	});

    async function addAccount(){
        try{
	        let result = await fetch(rootApi + "admin/api/addAccount", {
	            method: "POST",
	            headers: {
	                "Content-Type": "application/json",
	            },
	            body: JSON.stringify(addAccountDialog),
	        })

			if (!result.ok) {
                const message = await result.text();
                toast.warning(message, {
                    duration: 5000
                })
            }else{
				toast.success("Added", {
                    duration: 5000
                })
                await getAccounts();
                isAddOpen = false;
            }
	    }catch(error){
	        toast.error("Unknown error", {
				duration: 5000
			})
	    }
    }

	function openChange(i: number){
        changeAccountDialog = JSON.parse(JSON.stringify(accounts[i]));
		isChangeOpen = true;
    }

	function openDelete(i: number){
		deleteAccountDialog = accounts[i];
		isDeleteOpen = true;
	}

	async function onChange(){
        try {
            let result = await fetch(rootApi+"admin/api/changeAccount", {
                method: "POST",
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify(changeAccountDialog)
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
                await getAccounts();
                isChangeOpen = false;
            }
        } catch (error) {
            toast.error("Unknown error", {
				duration: 5000
			})
        }
    }

	async function onDelete(){
        try {
            let result = await fetch(rootApi+"admin/api/deleteAccount", {
                method: "POST",
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify(deleteAccountDialog)
            });

			if (!result.ok) {
                const message = await result.text();
                toast.warning(message, {
                    duration: 5000
                })
            }else{
				toast.success("Deleted", {
                    duration: 5000
                })
                await getAccounts();
                isDeleteOpen = false;
            }
        } catch (error) {
            toast.error("Unknown error", {
				duration: 5000
			})
        }
    }
	
	onMount(async () => {
		await getAccounts();
	});

    $inspect(accounts)
</script>

<div class="px-4 pb-4">
	<h1>Accounts</h1>

	<Dialog.Root bind:open={isAddOpen}>
		<Dialog.Trigger>
			<Button variant="secondary" class="my-4">Add</Button>
		</Dialog.Trigger>
		<Dialog.Content>
			<div class="break-words">
				<h2 class="mb-2">Add Account</h2>
				<Input
					bind:value={addAccountDialog.username}
					spellcheck="false"
					class="mb-2"
					placeholder="Username"
				></Input>
                <Input
					bind:value={addAccountDialog.password}
					spellcheck="false"
					class="mb-2"
                    type="password"
					placeholder="Password"
				></Input>

                <div class="flex items-center gap-3 mt-4">
                    <Checkbox id="isAdmin" bind:checked={addAccountDialog.isAdmin} />
                    <Label for="isAdmin">Admin privilege</Label>
                </div>
				<div class="flex items-center justify-end">
					<Button variant="secondary" onclick={async ()=>{await addAccount()}}>Add</Button>
				</div>
			</div>
		</Dialog.Content>
	</Dialog.Root>

	<!-- change user -->
	<Dialog.Root bind:open={isChangeOpen}>
		<Dialog.Content>
			<div class="break-words">
				<h2 class="mb-2">Change Account</h2>
				<Input
					bind:value={changeAccountDialog.username}
					spellcheck="false"
					class="mb-2"
					placeholder="Username"
					disabled
				></Input>
                <Input
					bind:value={changeAccountDialog.password}
					spellcheck="false"
					class="mb-2"
                    type="password"
					placeholder="Type to change password"
				></Input>

                <div class="flex items-center gap-3 mt-4">
                    <Checkbox id="isAdmin" bind:checked={changeAccountDialog.isAdmin} />
                    <Label for="isAdmin">Admin privilege</Label>
                </div>
				<div class="flex items-center justify-end">
					<Button variant="secondary" onclick={async ()=>{await onChange()}}>Save</Button>
				</div>
			</div>
		</Dialog.Content>
	</Dialog.Root>

	<Dialog.Root bind:open={isDeleteOpen}>
      <Dialog.Content>
        <div class="break-words">
            <p class="mb-2">Do you want to delete user <span class="text-muted-foreground">{deleteAccountDialog.username}</span> ?</p>  
            <div class="flex justify-end items-center">
                <Button variant="destructive" onclick={async ()=>{await onDelete()}}>Yes</Button>
            </div>
        </div>
      </Dialog.Content>
    </Dialog.Root>

    <div class="flex flex-col gap-2">
        {#each accounts as account, i}
		<DropdownMenu.Root>
                <DropdownMenu.Trigger>
                    <div class="flex items-center p-2 cursor-pointer rounded-sm hover:bg-accent">
                        <div class="flex gap-2">
							{#if account.isAdmin}
								<UserStar></UserStar>
							{:else}
								<User></User>
							{/if}
							<p>{account.username}</p>
            			</div>
                    </div>
                </DropdownMenu.Trigger>
                <DropdownMenu.Content align="start">
                    <DropdownMenu.Group>
                        <DropdownMenu.Item onclick={()=>{openChange(i)}}>Change</DropdownMenu.Item>
                        <DropdownMenu.Item onclick={()=>{openDelete(i)}}>Delete</DropdownMenu.Item>
                    </DropdownMenu.Group>
                </DropdownMenu.Content>
            </DropdownMenu.Root>
            
        {/each}
    </div>
</div>
