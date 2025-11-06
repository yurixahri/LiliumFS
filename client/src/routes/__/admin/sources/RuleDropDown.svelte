<script lang="ts">
    import * as DropdownMenu from "$lib/components/ui/dropdown-menu/index.js";
    import { Label } from "$lib/components/ui/label/index.js";
    import { Button } from "$lib/components/ui/button/index.js";
    import * as RadioGroup from "$lib/components/ui/radio-group/index.js";

    let {  value = $bindable(), accounts, label} = $props()
    let defaultAccessGroup = ["Anyone", "No one", "Any account"];
</script>

<DropdownMenu.Root>
    <DropdownMenu.Trigger class="w-1/3">
        <div class="flex flex-col justify-start gap-2">
            <Label class="text-muted-foreground">{label}</Label>
            <Button variant="secondary" class="block text-ellipsis overflow-hidden">{typeof value == "string" ? value : "Selected accounts"}</Button>
        </div>
    </DropdownMenu.Trigger>
    <DropdownMenu.Content align="start" onOpenAutoFocus={(event)=>{event.preventDefault()}}>
        <DropdownMenu.Group>
            <RadioGroup.Root bind:value={value}>
                {#each defaultAccessGroup as who}
                <DropdownMenu.Item class="px-1 py-0">
                    
                    <RadioGroup.Item value={who} id={who} class=""/>
                    <Label for={who} class="flex-1 py-1.5">{who}</Label>
                    
                </DropdownMenu.Item>
                {/each}
            </RadioGroup.Root>
        </DropdownMenu.Group>
        <DropdownMenu.Separator />
        <DropdownMenu.Label class="text-muted-foreground">Accounts</DropdownMenu.Label>
        <DropdownMenu.Group>
            <div class="flex flex-col gap-1">
                {#each accounts as account}
                    <div class="custom-checkbox-container">
                        <input type="checkbox" id={account.username} value={account.username} bind:group={value} />    
                        <label for={account.username}>{account.username}</label>
                    </div>
                {/each}
            </div>
        </DropdownMenu.Group>
    </DropdownMenu.Content>
</DropdownMenu.Root>

<style>
    .custom-checkbox-container {
        display: inline-flex;
        align-items: center;
        position: relative;
        cursor: pointer;
        -webkit-user-select: none;
        -moz-user-select: none;
        -ms-user-select: none;
        user-select: none;  
        padding: 2px 3px;
        border-radius: 3px;
    }
    .custom-checkbox-container:hover {
        background-color: var(--color-accent);
    }

    .custom-checkbox-container input {
        position: absolute;
        opacity: 0;
        cursor: pointer;
        height: 0;
        width: 0;
    }

    .custom-checkbox-container label {
        width: 100%;
        display: flex;
        align-items: center;
    }
     

    .custom-checkbox-container label::before {
        content: '';
        display: inline-block;
        width: 15px;
        height: 15px;
        border: 1px solid var(--color-sidebar-ring);
        border-radius: 50%;
        background-color: transparent;
        transition: background-color 0.3s, border-color 0.3s;
        margin-right: 10px;
    }

    .custom-checkbox-container label::after {
        content: '';
        position: absolute;
        display: none; 
        top: 10.5px;
        left: 7.3px;
        width: 6.5px;
        height: 6.5px;
        background: white;
        border-radius: 50%;
    }

    .custom-checkbox-container input:checked + label::after {
        display: block;
    }
</style>