<script lang="ts">
	import '../app.css';
	//import favicon from '$lib/assets/favicon.svg';
	import { background_image, icon } from '$lib/images/base64';

	document.documentElement.style.setProperty('--bg-image', `url("${background_image}")`);
	let { children } = $props();
	
</script>

<svelte:head>
	<link rel="icon" href={icon} />
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
