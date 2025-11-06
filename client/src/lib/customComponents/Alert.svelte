<script lang="ts">
	let {bindFunction, trigger_title, icon, description = "This action cannot be undone."} = $props();
	import * as AlertDialog from '$lib/components/ui/alert-dialog/index.js';
    import Button from "$lib/components/ui/button/button.svelte";

    let open = $state(false);
</script>

<AlertDialog.Root bind:open>
	<AlertDialog.Trigger >
		<Button variant="secondary" class="transition-all">
            {@render icon()}
            <span class="hidden sm:inline">{trigger_title}</span>
        </Button>
	</AlertDialog.Trigger>
	<AlertDialog.Content>
		<AlertDialog.Header>
			<AlertDialog.Title>Are you absolutely sure?</AlertDialog.Title>
			<AlertDialog.Description>
				{description}
			</AlertDialog.Description>
		</AlertDialog.Header>
		<AlertDialog.Footer>
			<AlertDialog.Cancel>Cancel</AlertDialog.Cancel>
			<AlertDialog.Action onclick={async ()=>{await bindFunction(); open = false}}>Continue</AlertDialog.Action>
		</AlertDialog.Footer>
	</AlertDialog.Content>
</AlertDialog.Root>
