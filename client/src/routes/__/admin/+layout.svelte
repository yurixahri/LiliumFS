<script lang="ts">
	import * as Sidebar from '$lib/components/ui/sidebar/index.js';
	import { Folders, Cog, HouseIcon, UserRoundCog, LogOut} from 'lucide-svelte';
	import { Toaster } from '$lib/components/ui/sonner/index.js';
	import { authenticate, authorize } from '$lib/auth/info';
	import { onMount } from 'svelte';
	import { goto } from '$app/navigation';
	import { rootApi } from '$lib/stores/globalValues';
	import Alert from '$lib/customComponents/Alert.svelte';

	let { children } = $props();
	const items = [
		{
			title: 'Home',
			url: '/__/admin',
			icon: HouseIcon
		},
		{
			title: 'Files',
			url: '/__/admin/sources',
			icon: Folders
		},
		{
			title: 'Accounts',
			url: '/__/admin/accounts',
			icon: UserRoundCog
		},
		{
			title: 'Settings',
			url: '/__/admin/settings',
			icon: Cog
		}
	];

	async function logout(){
        try{
            let result = await fetch(rootApi + "api/logout",{
                method: "GET",
            })
			goto('/__/admin/login');
        }catch{

        }
    }

	let isAuthorized = $state(false);
	onMount(async () => {
		let isLocalhost: boolean;

		if (typeof window !== 'undefined') isLocalhost = window.location.hostname === 'localhost';
		else isLocalhost = false;

		if (!isLocalhost) {
			if (!(await authenticate()) || !(await authorize())) {
				goto('/__/admin/login');
			}
		} else {
			goto('/__/admin');
			isAuthorized = true;
		}
	});


</script>

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

{#if isAuthorized}
	<Sidebar.Provider>
		<Sidebar.Root>
			<Sidebar.Content>
				<Sidebar.Group>
					<Sidebar.GroupLabel>Application</Sidebar.GroupLabel>
					<Sidebar.GroupContent>
						<Sidebar.Menu>
							{#each items as item (item.title)}
								<Sidebar.MenuItem>
									<Sidebar.MenuButton>
										{#snippet child({ props })}
											<a href={item.url} {...props}>
												<item.icon />
												<span>{item.title}</span>
											</a>
										{/snippet}
									</Sidebar.MenuButton>
								</Sidebar.MenuItem>
							{/each}
							<Sidebar.MenuItem class="mt-2 ml-2">
								<Alert bindFunction={logout} 
									trigger_title="Logout"
									description="You are about to logout.">
									{#snippet icon()}
										<LogOut/>
									{/snippet}
								</Alert>
							</Sidebar.MenuItem>
						</Sidebar.Menu>
					</Sidebar.GroupContent>
				</Sidebar.Group>
			</Sidebar.Content>
		</Sidebar.Root>
		<div class="flex h-screen w-full flex-col">
			<Sidebar.Trigger />
			{@render children?.()}
		</div>
	</Sidebar.Provider>
{/if}
