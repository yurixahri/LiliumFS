<script lang="ts">
	import { onMount } from 'svelte';
	import '../app.css';
	import { PUBLIC_staticURL } from '$env/static/public';
	//import favicon from '$lib/assets/favicon.svg';

	document.documentElement.style.setProperty('--bg-image', `url("${PUBLIC_staticURL}background.jpg")`);
	
	onMount(() => {
		let link = document.querySelector<HTMLLinkElement>('#runtime-colors');
		
		if (!link) {
			link = document.createElement('link');
			link.id = 'runtime-colors';
			link.rel = 'stylesheet';
			document.head.appendChild(link);
		}

		link.href = `${PUBLIC_staticURL}colors.css?v=${Date.now()}`;
	});
	let { children } = $props();
	
</script>

<svelte:head>
	<link rel="icon" href="{PUBLIC_staticURL}icon-100.png" />
</svelte:head>

<div class="bg">

</div>

{@render children?.()}

<style>
	.bg{
		position: fixed;
		height: 100vh;
		width: 100vw;
		background: linear-gradient(rgb(from var(--color-background) r g b / 0.90), rgb(from var(--color-background) r g b / 0.9)),
    var(--bg-image);;
		background-size: cover;
		background-repeat: no-repeat;
		background-position: center;
		z-index: -1;
	}

	:global(body){
		background: none;
	}
</style>
